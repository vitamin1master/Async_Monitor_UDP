#include "multitask_connection.h"
#include "stun_response.h"
#include <iostream>

//public:
multitask_connection::multitask_connection() : _socket(_io_service, udp::v4())
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

bool multitask_connection::start_checking(const std::vector<std::pair<std::string, int>>& servers_ports_list, const boost::function<void(std::vector<connection_info> completed_connections_info_list)>& func)
{
	_all_connections_stopped_handle = func;
	boost::function<void(std::shared_ptr<connection_test_packet>&)> connection_stop_handle(boost::bind(&multitask_connection::stop_connection, this, _1));
	
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
		auto packet = std::make_shared<connection_test_packet>(info, _io_service, connection_stop_handle);

		packet->timer.expires_from_now(boost::posix_time::milliseconds(interval*index));
		packet->timer.async_wait(boost::bind(&multitask_connection::connect_handle, this, packet, _1));
		
		_connections_list.push_back(packet);
		++index;
	}

	_io_service.run();

	return true;
}

//private:
void multitask_connection::connect_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error)
{
	if (error)
	{
		//Any promlems with _timer
		return;
	}

	start_connection(connection);
}

void multitask_connection::send_binding_request(std::shared_ptr<connection_test_packet>& connection)
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

		//2147483647 is int range
		connection->request_header.id[0] = rand() % 2147483647;
		connection->request_header.id[1] = rand() % 2147483647;
		connection->request_header.id[2] = rand() % 2147483647;
	}
	
	++connection->count_send_request;

	_socket.async_send_to(boost::asio::buffer(&connection->request_header, sizeof connection->request_header), connection->end_point, boost::bind(&multitask_connection::write_handle, this, connection, _1,_2));
}

void multitask_connection::start_connection(std::shared_ptr<connection_test_packet>& connection)
{
	connection->count_send_request = 0;
	send_binding_request(connection);
	
	connection->timer.expires_from_now(boost::posix_time::milliseconds(delay));
	connection->timer.async_wait(boost::bind(&multitask_connection::wait_handle, this, connection, _1));
}

void multitask_connection::wait_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error)
{
	if (error)
	{
		return;
	}
	if (connection->count_send_request < max_count_send_request)
	{
		if (!connection->stop_indicator)
		{
			send_binding_request(connection);

			connection->timer.expires_from_now(boost::posix_time::milliseconds(delay));
			connection->timer.async_wait(boost::bind(&multitask_connection::wait_handle, this, connection, _1));
		}
	}
	else
	{
		connection->stop_indicator = true;
		connection->connection_stop_handler(connection);
		
		boost::system::error_code ec;
		connection->timer.cancel(ec);
		if(ec)
		{
			std::cerr << ec.message() << std::endl;
		}
	}
}

void multitask_connection::write_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes)
{
	if(error)
	{
		std::cerr << error.message() << std::endl;
		return;
	}
	do_read(connection);
}

void multitask_connection::stop_connection(std::shared_ptr<connection_test_packet>& connection)
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
		_all_connections_stopped_handle(_completed_connections_info_list);
	}
}

void multitask_connection::read_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes)
{
	connection->read_indicator = false;
	if(error)
	{
		std::cerr << error.message() << std::endl;

		return;
	}

	auto it=std::find_if(_connections_list.begin(), _connections_list.end(), std::bind(&connection_test_packet::isConnectSougth, std::placeholders::_1, connection->remote_end_point));
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
	char* mapped_address = new char[20];
	snprintf(mapped_address, 20, "%d.%d.%d.%d:%hu", std::abs(response_struct.mappedaddr_attr.id[0] ^ 0x21), std::abs(response_struct.mappedaddr_attr.id[1] ^ 0x12),
		std::abs(response_struct.mappedaddr_attr.id[2] ^ 0xA4), std::abs(response_struct.mappedaddr_attr.id[3] ^ 0x42), std::abs(port));

	std::string intermediateString(mapped_address);
	it->get()->info.returned_ip_port = intermediateString;

	server_is_active(*it);

	delete[]mapped_address;
	mapped_address = nullptr;
}

void multitask_connection::do_read(std::shared_ptr<connection_test_packet>& connection)
{
	if(connection->read_indicator)
	{
		return;
	}

	if(!_connections_list.size())
	{
		return;
	}

	if(connection->response_buf)
	{
		delete[]connection->response_buf;
	}

	connection->response_buf = new char[max_length_response];
	_socket.async_receive_from(boost::asio::buffer(connection->response_buf, max_length_response), connection->remote_end_point,
		boost::bind(&multitask_connection::read_handle, this, connection,_1, _2));
	connection->read_indicator = true;
}

void multitask_connection::server_is_active(std::shared_ptr<connection_test_packet>& connection)
{
	boost::system::error_code ec;
	connection->timer.cancel(ec);
	if(ec)
	{
		std::cerr << ec.message() << std::endl;
	}
	connection->stop_indicator = true;
	connection->info.stun_server_is_active = true;

	connection->connection_stop_handler(connection);
}

bool multitask_connection::check_response(stun_response& response_struct, const std::shared_ptr<connection_test_packet>& connection, const size_t& bytes)
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
	for (auto i = 0; i < response_data_len&&i < bytes - msg_hdr_length; i += ntohs(response_struct.mappedaddr_attr.length))
	{
		if (i)
		{
			response_struct.mappedaddr_attr.type = 0;

			char* point = nullptr;
			point = connection->response_buf + msg_hdr_length + i;
			memcpy(&response_struct.mappedaddr_attr, point, bytes - msg_hdr_length - i);
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