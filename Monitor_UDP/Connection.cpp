#include "Connection.h"
//#include "IpHeader.h"
#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif


#include <boost/bind.hpp>
#include <iostream>
#include <stdio.h>

using boost::asio::ip::tcp;

#define binding1(x) boost::bind(&Connection::x,shared_from_this())
#define binding2(x,y) boost::bind(&Connection::x,shared_from_this(),y)
#define binding3(x,y,z) boost::bind(&Connection::x,shared_from_this(),y,z)

//private:
void Connection::sendBindingRequest()
{
	*reinterpret_cast<short *>(&_requestBuffer[0]) = htons(0x0001);
	*reinterpret_cast<short *>(&_requestBuffer[2]) = htons(0x0000);
	*reinterpret_cast<int *>(&_requestBuffer[4]) = htonl(0x2112A442);

	*reinterpret_cast<int *>(&_requestBuffer[8]) = htonl(0x63c7117e);   // transacation ID 
	*reinterpret_cast<int *>(&_requestBuffer[12]) = htonl(0x0714278f);
	*reinterpret_cast<int *>(&_requestBuffer[16]) = htonl(0x5ded3221);

	//IpHeader* iph = new IpHeader;
	//iph->iph_stun_method = static_cast<unsigned short>(0x0001);
	//iph->iph_mgs_length = htons(0x0000);
	//iph->iph_magic_cookie = htonl(0x2112A442);

	//iph->iph_transaction_id_part1 = htonl(0x63c7117e);   // transacation ID 
	//iph->iph_transaction_id_part2 = htonl(0x0714278f);
	//iph->iph_transaction_id_part3 = htonl(0x5ded3221);

	char a[2];
	//a[0] = iph->iph_stun_method;
	char b[2];

	_socket->async_send(boost::asio::buffer(_requestBuffer, maxLengthRequest), binding3(writeHandle, _1, _2));

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
	_socket->async_receive(boost::asio::buffer(_responseBuffer, maxLengthResponse),
		binding3(readHandle, _1, _2));
}
void Connection::readHandle(const boost::system::error_code& error, size_t bytes)
{
	if (*reinterpret_cast<short *>(&_responseBuffer[0]) == htons(0x0101))
	{
		short attr_length = 0;
		short attr_type;
		for (int i = maxLengthRequest; i < bytes; i += attr_length + 4)
		{
			attr_type = ntohs(*reinterpret_cast<short *>(&_responseBuffer[i]));
			attr_length = htons(*reinterpret_cast<short *>(&_responseBuffer[i + 2]));
			if (attr_type == 0x0020)
			{
				short port = ntohs(*reinterpret_cast<short *>(&_responseBuffer[i + 6]));
				port ^= 0x2112;
				char str[maxLengthResponse];
				snprintf(str, maxLengthResponse, "%d.%d.%d.%d:%d", std::abs(_responseBuffer[i + 8] ^ 0x21), std::abs(_responseBuffer[i + 9] ^ 0x12),
					std::abs(_responseBuffer[i + 10] ^ 0xA4), std::abs(_responseBuffer[i + 11] ^ 0x42), std::abs(port));
				std::string intermediateString(str);
				ReturnedIpPort = intermediateString;
				stunServerisActive();
				break;
			}
		}
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
	std::shared_ptr< udp::socket> soc_ptr(new udp::socket(io_service));
	_socket.swap(soc_ptr);

	IndexConnection = indexConnection;
	StopIndicator = false;
	_readIndicator = false;
	ServerId = serverId;
	ServerPort = serverPort;
	StunServerIsActive = false;
	ReturnedIpPort = "";

	std::shared_ptr<boost::asio::deadline_timer> time_ptr(new boost::asio::deadline_timer(io_service));
	_timer.swap(time_ptr);

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
	_socket.reset();
	_timer.reset();
}