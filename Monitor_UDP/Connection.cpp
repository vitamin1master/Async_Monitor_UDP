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

//private:
void Connection::sendBindingRequest()
{
	stun_header iph(htons(1));
	stun_request req = iph.get_request();

	_socket->async_send(boost::asio::buffer(&req, sizeof iph.get_request()), binding3(writeHandle, _1, _2));
}
void Connection::writeHandle(const boost::system::error_code& error, size_t bytes)
{
	if (error)
	{
		std::cout << error.message() << std::endl;
		std::error_code e(static_cast<int>(std::errc::operation_canceled),
			std::generic_category());
		throw new std::system_error(e);
	}
	doRead();
}

void Connection::doRead()
{
	if (_readIndicator)
	{
		return;
	}
	_readIndicator = true;
	_socket->async_receive(boost::asio::buffer(&_responseBuffer, maxLengthResponse),
		binding3(readHandle, _1, _2));
}
void Connection::readHandle(const boost::system::error_code& error, size_t bytes)
{
	stun_header buf(_responseBuffer, bytes);
	char* mapped_address = buf.mapped_address();
	if (mapped_address != nullptr)
	{
		std::string intermediateString(mapped_address);
		ReturnedIpPort = intermediateString;
		stunServerisActive();
	}
	_readIndicator = false;
}
void Connection::stunServerisActive()
{
	_timer->cancel();
	StopIndicator = true;
	StunServerIsActive = true;
	std::cout << ServerId << " is active" << std::endl;
}
void Connection::waitHandle(const boost::system::error_code error)
{
	if (error)
	{
		return;
	}
	if (StunServerIsActive)
	{
		return;
	}
	if (_countSendRequest < maxCountSendRequest)
	{
		if (!StopIndicator)
		{
			_countSendRequest++;
			try
			{
				sendBindingRequest();
			}
			catch (std::system_error& e)
			{
				std::cout << e.code() << std::endl;
			}
			_timer->expires_from_now(boost::posix_time::milliseconds(delay));
			_timer->async_wait(binding2(waitHandle, _1));
		}
		else
		{
			if (!StunServerIsActive)
			{
				ReturnedIpPort = "Invalid response";
			}
			_timer->cancel();
			std::cout << "CLose connection" << std::endl;
		}
	}
	else
	{
		_timer->cancel();
		StopIndicator = true;
		std::cout << ServerId << " does not respond" << std::endl;
		std::cout << "CLose connection" << std::endl;
	}
}
void Connection::connectHandle(const boost::system::error_code error)
{
	if (error)
	{
		std::cout << "Error" << std::endl;
		return;
	}
	StartConnection();
}


//public:

void Connection::StartConnection()
{
	_countSendRequest = 1;
	try
	{
		sendBindingRequest();
	}
	catch (std::system_error& e)
	{
		std::cout << e.code() << std::endl;
	}
	_timer->expires_from_now(boost::posix_time::milliseconds(delay));
	_timer->async_wait(binding2(waitHandle, _1));
}

std::shared_ptr<udp::socket> Connection::Socket()
{
	return _socket;
}


Connection::Connection(boost::asio::io_service& io_service, int indexConnection, std::string serverId, int serverPort)
{
	_socket.reset(new udp::socket(io_service));

	IndexConnection = indexConnection;
	StopIndicator = false;
	_readIndicator = false;
	ServerId = serverId;
	ServerPort = serverPort;
	StunServerIsActive = false;
	ReturnedIpPort = "";

	_timer.reset(new boost::asio::deadline_timer(io_service));

	_countSendRequest = 0;
}

void Connection::Connect()
{
	char* server = new char[ServerId.length() + 1];
	snprintf(server, ServerId.length() + 1, ServerId.c_str());
	boost::asio::ip::udp::endpoint endPoint_udp(boost::asio::ip::address::from_string(server), ServerPort);
	_socket->async_connect(endPoint_udp, boost::bind(&Connection::connectHandle, shared_from_this(), _1));
}


Connection::~Connection()
{
}