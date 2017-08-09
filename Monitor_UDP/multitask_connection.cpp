#include "multitask_connection.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "stun_response.h"
#include <iostream>
#include <thread>

//public:
multitask_connection::multitask_connection(std::vector<std::pair<std::string, int>> servers_ports_list,
	boost::function<void(std::vector<connection_info> completed_connections_info_list)> func) : _socket(_io_service), all_connections_stopped_handle(func)
{
	boost::function<void(std::shared_ptr<connection_test_packet>)> connection_stop_handle(boost::bind(&multitask_connection::stop_connection, this, _1));
	int index = 0;
	for (auto it : servers_ports_list)
	{
		connection_info info{ it.first,it.second, false, index};
		std::shared_ptr<connection_test_packet> packet;
		packet.reset(new connection_test_packet(info, _io_service, connection_stop_handle));
		connections_list.push_back(packet);
		++index;
	}
}

multitask_connection::~multitask_connection()
{
}

void multitask_connection::connect()
{
	for (auto &it : connections_list)
	{
		char* server = new char[it->info.server_id.length() + 1];
		snprintf(server, it->info.server_id.length() + 1, it->info.server_id.c_str());
		udp::endpoint endPoint_udp(boost::asio::ip::address::from_string(server), it->info.server_port);
		_socket.async_connect(endPoint_udp, boost::bind(&multitask_connection::connect_handle, this, it, _1));
		std::this_thread::sleep_for(std::chrono::milliseconds(_interval));
	}
	_io_service.run();
}

//private:
void multitask_connection::connect_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error)
{
	if (error)
	{
		//std::cout << "Error" << std::endl;
		return;
	}
	start_connection(connection);
}

void multitask_connection::send_binding_request(std::shared_ptr<connection_test_packet>& connection)
{
	connection->request_header.msg_type = msg_type_binding_request;
	connection->request_header.data_len = 0;
	connection->request_header.magic_cookie = fixed_magic_cookie;
	//2147483647 is int range
	connection->request_header.id[0] = rand() % 2147483647;
	connection->request_header.id[1] = rand() % 2147483647;
	connection->request_header.id[2] = rand() % 2147483647;
	_socket.async_send_to(boost::asio::buffer(&connection->request_header, sizeof connection->request_header), connection->end_point, boost::bind(&multitask_connection::write_handle, this, connection, _1,_2));
}

void multitask_connection::start_connection(std::shared_ptr<connection_test_packet>& connection)
{
	connection->connect_indicator = true;
	connection->count_send_request = 1;
	try
	{
		send_binding_request(connection);
	}
	catch (...)
	{
		//std::cout << e.code() << std::endl;
	}
	connection->timer.expires_from_now(boost::posix_time::milliseconds(_delay));
	connection->timer.async_wait(boost::bind(&multitask_connection::wait_handle, this, connection, _1));
}

void multitask_connection::wait_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error)
{
	if (error)
	{
		return;
	}
	if (connection->count_send_request < _max_count_send_request)
	{
		if (!connection->stop_indicator)
		{
			++connection->count_send_request;
			try
			{
				send_binding_request(connection);
			}
			catch (...)
			{
				//std::cout << e.code() << std::endl;
			}
			connection->timer.expires_from_now(boost::posix_time::milliseconds(_delay));
			connection->timer.async_wait(boost::bind(&multitask_connection::wait_handle, this, connection, _1));
		}
		else
		{
			if (!connection->info.stun_server_is_active)
			{
				connection->info.returned_ip_port = "Invalid response";
			}
			connection->timer.cancel();
			//std::cout << "CLose connection" << std::endl;
			connection->connection_stop_handler(connection);
		}
	}
	else
	{
		connection->timer.cancel();
		connection->stop_indicator = true;
		//std::cout << server_id << " does not respond" << std::endl;
		//std::cout << "CLose connection" << std::endl;
		connection->connection_stop_handler(connection);
	}
}

void multitask_connection::write_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes)
{
	if(!error)
	{
		do_read(connection);
	}
}

void multitask_connection::stop_connection(std::shared_ptr<connection_test_packet>& connection)
{
	auto iterator = std::find(connections_list.begin(), connections_list.end(), connection);
	if (iterator != connections_list.end())
		connections_list.erase(iterator);
	connection_info c_info{ connection->info.server_id, 0, connection->info.stun_server_is_active, connection->info.index_Connection, connection->info.returned_ip_port };
	completed_connections_info_list.push_back(c_info);
	if (!connections_list.size())
	{
		all_connections_stopped_handle(completed_connections_info_list);
	}
}

void multitask_connection::read_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes)
{
	if(error)
	{
		return;
	}
	connection->read_indicator = false;
	stun_response response_struct;
	memcpy(&response_struct, connection->response_buffer, bytes);
	if (response_struct.msg_type != msg_type_binding_response)
	{
		return;
	}
	if (connection->request_header.id[0] != response_struct.transaction_id[0] &&
		connection->request_header.id[1] != response_struct.transaction_id[1] &&
		connection->request_header.id[2] != response_struct.transaction_id[2])
	{
		return;
	}
	uint32_t response_data_len = ntohs(response_struct.data_len);
	for (auto i = 0; i < response_data_len&&i < bytes - msg_hdr_length; i += ntohs(response_struct.mappedaddr_attr.length))
	{
		if (!i)
		{
			response_struct.mappedaddr_attr.type = 0;
			char * point = &connection->response_buffer[msg_hdr_length + i];
			memcpy(&response_struct.mappedaddr_attr, point, bytes - msg_hdr_length - i);
		}
		if (response_struct.mappedaddr_attr.type == attr_type_xor_mappaddr)
		{
			break;
		}
	}
	if (response_struct.mappedaddr_attr.type != attr_type_xor_mappaddr)
	{
		return;
	}
	//IPv4 only
	if (response_struct.mappedaddr_attr.family != attr_family_ipv4)
	{
		return;
	}
	uint16_t port = ntohs(response_struct.mappedaddr_attr.port);
	port ^= 0x2112;
	char* mapped_address = new char[20];
	snprintf(mapped_address, 20, "%d.%d.%d.%d:%hu", std::abs(response_struct.mappedaddr_attr.id[0] ^ 0x21), std::abs(response_struct.mappedaddr_attr.id[1] ^ 0x12),
		std::abs(response_struct.mappedaddr_attr.id[2] ^ 0xA4), std::abs(response_struct.mappedaddr_attr.id[3] ^ 0x42), std::abs(port));

	std::string intermediateString(mapped_address);
	connection->info.returned_ip_port = intermediateString;
	server_is_active(connection);
}

void multitask_connection::do_read(std::shared_ptr<connection_test_packet>& connection)
{
	if(!connection->read_indicator)
	{
		_socket.async_receive_from(boost::asio::buffer(connection->response_buffer, connection->_max_length_response), connection->end_point,
			boost::bind(&multitask_connection::read_handle, this, connection, _1, _2));
		connection->read_indicator = true;
	}
}

void multitask_connection::server_is_active(std::shared_ptr<connection_test_packet>& connection)
{
	connection->timer.cancel();
	connection->stop_indicator = true;
	connection->info.stun_server_is_active = true;
	//std::cout << server_id << " is active" << std::endl;
	connection->connection_stop_handler(connection);
}
