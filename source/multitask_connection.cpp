#include "headers/multitask_connection.h"
#include <iostream>

//public:
multitask_connection::multitask_connection(imonitor* monitor,
										   int period_sending_request_ms,
										   int max_number_request_sent) : _socket(_io_service, udp::v4()),
																		  _max_number_request_sent(max_number_request_sent),
																		  _period_sending_request_ms(period_sending_request_ms),
																		  _monitor(monitor)
{
}

multitask_connection::~multitask_connection()
{
	_connections_list.clear();
	_completed_connections_info_list.clear();
	
	boost::system::error_code ec;
	if (_socket.is_open())
	{
		_socket.close(ec);
		if(ec)
		{
			std::cerr << ec.message() << std::endl;
		}
	}
}

bool multitask_connection::start_checking(const std::vector<std::pair<std::string, int>>& servers_ports_list)
{
	boost::system::error_code ec;
	if (!_socket.is_open())
	{
		_socket.open(udp::v4(), ec);
		if (ec)
		{
			std::cerr << ec.message() << std::endl;
			return false;
		}
	}

	auto index = 0;
	for (auto &it : servers_ports_list)
	{
		//Create packet
		connection_info info{ it.first,it.second, false, index };
		auto packet = std::make_shared<connection_test_packet>(info, _io_service);

		packet->timer.expires_from_now(boost::posix_time::milliseconds(interval*index));
		packet->timer.async_wait(boost::bind(&multitask_connection::connect_handle, this, packet, _1));
		
		_connections_list.push_back(packet);
		++index;
	}

	_io_service.run();

	return true;
}

//private:
void multitask_connection::connect_handle(const std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error)
{
	if (error)
	{
		//Any promlems with _timer
		return;
	}

	start_connection(connection);
}

void multitask_connection::send_binding_request(const std::shared_ptr<connection_test_packet>& connection)
{
	if(!_connections_list.size())
	{
		return;
	}
	if (!connection->count_send_request)
	{
		connection->request_header.msg_type = msg_type_binding_request;
		connection->request_header.data_len = 0;
		connection->request_header.magic_cookie = fixed_magic_cookie;

		connection->request_header.id[0] = _rand_dev();
		connection->request_header.id[1] = _rand_dev();
		connection->request_header.id[2] = _rand_dev();
	}
	
	++connection->count_send_request;

	_socket.async_send_to(boost::asio::buffer(&connection->request_header, sizeof connection->request_header), connection->end_point, boost::bind(&multitask_connection::write_handle, this, connection, _1,_2));
}

void multitask_connection::start_connection(const std::shared_ptr<connection_test_packet>& connection)
{
	connection->count_send_request = 0;
	send_binding_request(connection);
	
	connection->timer.expires_from_now(boost::posix_time::milliseconds(_period_sending_request_ms));
	connection->timer.async_wait(boost::bind(&multitask_connection::wait_handle, this, connection, _1));
}

void multitask_connection::wait_handle(const std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error)
{
	if (error)
	{
		return;
	}
	if (connection->count_send_request < _max_number_request_sent)
	{
		if (!connection->stop_indicator)
		{
			send_binding_request(connection);

			connection->timer.expires_from_now(boost::posix_time::milliseconds(_period_sending_request_ms));
			connection->timer.async_wait(boost::bind(&multitask_connection::wait_handle, this, connection, _1));
		}
	}
	else
	{
		connection->stop_indicator = true;
		stop_connection(connection);
	}
}

void multitask_connection::write_handle(const std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes)
{
	if(error)
	{
		std::cerr << error.message() << std::endl;
		return;
	}
	do_read(connection);
}

void multitask_connection::stop_connection(const std::shared_ptr<const connection_test_packet>& connection)
{
	auto iterator = std::find(_connections_list.begin(), _connections_list.end(), connection);
	if (iterator == _connections_list.end())
	{
		std::cerr << "No object found connection if vector connections_list" << std::endl;
		return;
	}

	connection_info info = connection->info;
	_completed_connections_info_list.push_back(info);

	_connections_list.erase(iterator);
	if (!_connections_list.size())
	{
		_io_service.stop();
		_monitor->verification_result_monitoring(_completed_connections_info_list);
	}
}

void multitask_connection::read_handle(const std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes)
{
	connection->read_indicator = false;
	if(error)
	{
		std::cerr << error.message() << std::endl;

		return;
	}

	auto it=std::find_if(_connections_list.begin(), _connections_list.end(), std::bind(&connection_test_packet::is_connect_sougth, std::placeholders::_1, connection->remote_end_point));
	if(it==_connections_list.end())
	{
		//A connection to the remote_end_point has already established, or message has come from an unknown source
		return;
	}

	stun_response response_struct;
	memcpy(&response_struct, connection->response_buf, bytes);

	check_response(response_struct, *it, bytes);
	
	uint16_t port = ntohs(response_struct.mappedaddr_attr.port);
	port ^= 0x2112;
    int p=std::abs(port);
	const int max_len_addr=22;
	char mapped_address[max_len_addr];

	snprintf(mapped_address, max_len_addr, "%d.%d.%d.%d:%d", std::abs(response_struct.mappedaddr_attr.id[0] ^ 0x21), std::abs(response_struct.mappedaddr_attr.id[1] ^ 0x12),
		std::abs(response_struct.mappedaddr_attr.id[2] ^ 0xA4), std::abs(response_struct.mappedaddr_attr.id[3] ^ 0x42), p);

	std::string intermediateString(mapped_address);
	it->get()->info.returned_ip_port = intermediateString;

	server_is_active(*it);
}

void multitask_connection::do_read(const std::shared_ptr<connection_test_packet>& connection)
{
	if(connection->read_indicator)
	{
		return;
	}

	if(!_connections_list.size())
	{
		return;
	}

	_socket.async_receive_from(boost::asio::buffer(connection->response_buf, connection_test_packet::max_length_response), connection->remote_end_point,
		boost::bind(&multitask_connection::read_handle, this, connection,_1, _2));
	connection->read_indicator = true;
}

void multitask_connection::server_is_active(const std::shared_ptr<connection_test_packet>& connection)
{
	boost::system::error_code ec;
	connection->timer.cancel(ec);
	if(ec)
	{
		std::cerr << ec.message() << std::endl;
	}
	connection->stop_indicator = true;
	connection->info.stun_server_is_active = true;

	stop_connection(connection);
}

bool multitask_connection::check_response(stun_response& response_struct, const std::shared_ptr<const connection_test_packet>& connection, const size_t& bytes) const
{
	if (response_struct.msg_type != msg_type_binding_response)
	{
		std::cerr << "The received response has a another type" << std::endl;
		return false;
	}

	if (connection->request_header.id[0] != response_struct.transaction_id[0] &&
		connection->request_header.id[1] != response_struct.transaction_id[1] &&
		connection->request_header.id[2] != response_struct.transaction_id[2])
	{
		//The received response has a another transaction from
		return false;
	}

	//Look for the response type==mappedaddr_attr
	uint32_t response_data_len = ntohs(response_struct.data_len);
	for (unsigned int i = 0; i < response_data_len&&i < bytes - msg_hdr_length; i += ntohs(response_struct.mappedaddr_attr.length))
	{
		if (i) {
			response_struct.mappedaddr_attr.type = 0;

			char cut_resp_buf[attr_len_xor_mappaddr];
			for (int k = 0; k <attr_len_xor_mappaddr ; k++)
			{
				cut_resp_buf[k]=connection->response_buf[k+i+msg_hdr_length];
			}
			memcpy(&response_struct.mappedaddr_attr, cut_resp_buf, attr_len_xor_mappaddr);
		}

		if (response_struct.mappedaddr_attr.type == attr_type_xor_mappaddr)
		{
			break;
		}
	}

	if (response_struct.mappedaddr_attr.type != attr_type_xor_mappaddr)
	{
		//The received response has't mapped address attribute
		return false;
	}
	//IPv4 only
	if (response_struct.mappedaddr_attr.family != attr_family_ipv4)
	{
		//The received response has attribute family IPv4
		return false;
	}

	return true;
}
