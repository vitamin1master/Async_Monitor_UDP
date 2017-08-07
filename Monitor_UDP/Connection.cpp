#include "Connection.h"
#include <boost/bind.hpp>
#include <iostream>
#include <stdio.h>
#include <boost/function.hpp>
#include "stun_response.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

//public:

Connection::Connection(boost::asio::io_service& io_service, int indexConnection, std::string serverId, int serverPort, boost::function<void(std::shared_ptr<Connection>)> func)
{
	_socket.reset(new udp::socket(io_service));
	_connection_stop_handler = func;

	index_Connection = indexConnection;
	stop_indicator = false;
	_read_indicator = false;
	server_id = serverId;
	server_port = serverPort;
	stun_server_is_active = false;
	returned_ip_port = "";

	_timer.reset(new boost::asio::deadline_timer(io_service));

	_count_send_request = 0;
}

Connection::~Connection()
{
}

std::shared_ptr<udp::socket> Connection::socket()
{
	return _socket;
}

//private:

void Connection::connect()
{
	char* server = new char[server_id.length() + 1];
	snprintf(server, server_id.length() + 1, server_id.c_str());
	udp::endpoint endPoint_udp(boost::asio::ip::address::from_string(server), server_port);
	_socket->async_connect(endPoint_udp, boost::bind(&Connection::connect_handle, shared_from_this(), _1));
}

void Connection::start_connection()
{
	_count_send_request = 1;
	try
	{
		send_binding_request();
	}
	catch (...)
	{
		//std::cout << e.code() << std::endl;
	}
	_timer->expires_from_now(boost::posix_time::milliseconds(_delay));
	_timer->async_wait(boost::bind(&Connection::wait_handle,shared_from_this(),_1));
}

void Connection::send_binding_request()
{
	_request_struct.msg_type = msg_type_binding_request;
	_request_struct.data_len = 0;
	_request_struct.magic_cookie = fixed_magic_cookie;
	//2147483647 is int range
	_request_struct.id[0] = rand() % 2147483647;
	_request_struct.id[1] = rand() % 2147483647;
	_request_struct.id[2] = rand() % 2147483647;
	_socket->async_send(boost::asio::buffer(&_request_struct,sizeof _request_struct), boost::bind(&Connection::write_handle, shared_from_this(), _1,_2));
}
void Connection::write_handle(const boost::system::error_code& error, size_t bytes)
{
	if (error)
	{
		//std::cout << error.message() << std::endl;
		//binding request will be sent again from he wait_handle
		return;
	}
	do_read();
}

void Connection::do_read()
{
	if (_read_indicator)
	{
		return;
	}
	_read_indicator = true;
	_socket->async_receive(boost::asio::buffer(&_response_buffer, _max_length_response),
		boost::bind(&Connection::read_handle, shared_from_this(),_1,_2));
}
void Connection::read_handle(const boost::system::error_code& error, size_t bytes)
{
	stun_response response_struct;
	memcpy(&response_struct, _response_buffer, bytes);
	if (response_struct.msg_type != msg_type_binding_response)
	{
		_read_indicator = false;
		return;
	}
	if (_request_struct.id[0] != response_struct.transaction_id[0] &&
		_request_struct.id[1] != response_struct.transaction_id[1] &&
		_request_struct.id[2] != response_struct.transaction_id[2])
	{
		_read_indicator = false;
		return;
	}
	uint32_t response_data_len = ntohs(response_struct.data_len);
	for (auto i=0; i < response_data_len&&i < bytes - msg_hdr_length; i += ntohs(response_struct.mappedaddr_attr.length))
	{
		if (!i)
		{
			response_struct.mappedaddr_attr.type = 0;
			char * point = &_response_buffer[msg_hdr_length + i];
			memcpy(&response_struct.mappedaddr_attr, point, bytes - msg_hdr_length - i);
		}
		if(response_struct.mappedaddr_attr.type== attr_type_xor_mappaddr)
		{
			break;
		}
	}
	if (response_struct.mappedaddr_attr.type !=attr_type_xor_mappaddr)
	{
		_read_indicator = false;
		return;
	}
	//IPv4 only
	if(response_struct.mappedaddr_attr.family!=attr_family_ipv4)
	{
		_read_indicator = false;
		return;
	}
	uint16_t port = ntohs(response_struct.mappedaddr_attr.port);
	port ^= 0x2112;
	char* mapped_address = new char[20];
	snprintf(mapped_address, 20, "%d.%d.%d.%d:%hu", std::abs(response_struct.mappedaddr_attr.id[0] ^ 0x21), std::abs(response_struct.mappedaddr_attr.id[1] ^ 0x12),
		std::abs(response_struct.mappedaddr_attr.id[2] ^ 0xA4), std::abs(response_struct.mappedaddr_attr.id[3] ^ 0x42), std::abs(port));

	std::string intermediateString(mapped_address);
	returned_ip_port = intermediateString;
	stun_server_isActive();
}
void Connection::stun_server_isActive()
{
	_timer->cancel();
	stop_indicator = true;
	stun_server_is_active = true;
	//std::cout << server_id << " is active" << std::endl;
	_connection_stop_handler(shared_from_this());
}
void Connection::wait_handle(const boost::system::error_code error)
{
	if (error)
	{
		return;
	}
	if (_count_send_request < _max_count_send_request)
	{
		if (!stop_indicator)
		{
			_count_send_request++;
			try
			{
				send_binding_request();
			}
			catch (...)
			{
				//std::cout << e.code() << std::endl;
			}
			_timer->expires_from_now(boost::posix_time::milliseconds(_delay));
			_timer->async_wait(boost::bind(&Connection::wait_handle, shared_from_this(), _1));
		}
		else
		{
			if (!stun_server_is_active)
			{
				returned_ip_port = "Invalid response";
			}
			_timer->cancel();
			//std::cout << "CLose connection" << std::endl;
			_connection_stop_handler(shared_from_this());
		}
	}
	else
	{
		_timer->cancel();
		stop_indicator = true;
		//std::cout << server_id << " does not respond" << std::endl;
		//std::cout << "CLose connection" << std::endl;
		_connection_stop_handler(shared_from_this());
	}
}
void Connection::connect_handle(const boost::system::error_code error)
{
	if (error)
	{
		//std::cout << "Error" << std::endl;
		return;
	}
	start_connection();
}
