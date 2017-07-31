#include "Connection.h"
#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif


#include <boost/bind.hpp>
#include <iostream>
#include <stdio.h>
#include "stun_header.h"

using boost::asio::ip::tcp;

#define binding1(x) boost::bind(&Connection::x,shared_from_this())
#define binding2(x,y) boost::bind(&Connection::x,shared_from_this(),y)
#define binding3(x,y,z) boost::bind(&Connection::x,shared_from_this(),y,z)

//public:

Connection::Connection(boost::asio::io_service& io_service, int indexConnection, std::string serverId, int serverPort, Monitor* monitor)
{
	_socket.reset(new udp::socket(io_service));
	_monitor = monitor;

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

std::shared_ptr<udp::socket> Connection::Socket()
{
	return _socket;
}

//private:

void Connection::Connect()
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
	catch (std::system_error& e)
	{
		std::cout << e.code() << std::endl;
	}
	_timer->expires_from_now(boost::posix_time::milliseconds(_delay));
	_timer->async_wait(binding2(wait_handle, _1));
}

void Connection::send_binding_request()
{
	stun_header iph(htons(1));
	stun_request req = iph.get_request();

	_socket->async_send(boost::asio::buffer(&req, sizeof iph.get_request()), binding3(write_handle, _1, _2));
}
void Connection::write_handle(const boost::system::error_code& error, size_t bytes)
{
	if (error)
	{
		std::cout << error.message() << std::endl;
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
		binding3(read_handle, _1, _2));
}
void Connection::read_handle(const boost::system::error_code& error, size_t bytes)
{
	stun_header buf(_response_buffer, bytes);
	char* mapped_address = buf.mapped_address();
	if (mapped_address != nullptr)
	{
		std::string intermediateString(mapped_address);
		returned_ip_port = intermediateString;
		stun_server_isActive();
	}
	_read_indicator = false;
}
void Connection::stun_server_isActive()
{
	_timer->cancel();
	stop_indicator = true;
	stun_server_is_active = true;
	std::cout << server_id << " is active" << std::endl;
}
void Connection::wait_handle(const boost::system::error_code error)
{
	if (error)
	{
		_monitor->stop_connection();
	}
	if (stun_server_is_active)
	{
		return;
	}
	if (_count_send_request < _max_length_request)
	{
		if (!stop_indicator)
		{
			_count_send_request++;
			try
			{
				send_binding_request();
			}
			catch (std::system_error& e)
			{
				std::cout << e.code() << std::endl;
			}
			_timer->expires_from_now(boost::posix_time::milliseconds(_delay));
			_timer->async_wait(binding2(wait_handle, _1));
		}
		else
		{
			if (!stun_server_is_active)
			{
				returned_ip_port = "Invalid response";
			}
			_timer->cancel();
			std::cout << "CLose connection" << std::endl;
			_monitor->stop_connection();
		}
	}
	else
	{
		_timer->cancel();
		stop_indicator = true;
		std::cout << server_id << " does not respond" << std::endl;
		std::cout << "CLose connection" << std::endl;
		_monitor->stop_connection();
	}
}
void Connection::connect_handle(const boost::system::error_code error)
{
	if (error)
	{
		std::cout << "Error" << std::endl;
		return;
	}
	start_connection();
}