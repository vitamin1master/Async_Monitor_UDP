#pragma once

#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif

#include <boost/asio.hpp>
#include "stun_request.h"
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
class Connection;



class Connection : public std::enable_shared_from_this<Connection>
{
public:
	Connection(boost::asio::io_service& io_service, int indexConnection, std::string serverId, int serverPort);
	~Connection();
	void Connect();

	int IndexConnection;
	bool StunServerIsActive;
	bool StopIndicator;
	std::string ServerId;
	int ServerPort;
	std::string ReturnedIpPort;
	void StartConnection();
	std::shared_ptr< udp::socket> Socket();

private:
	static const int maxLengthResponse = 128;
	static const int maxLengthRequest = 20;
	static const int delay = 1000;
	static const int maxCountSendRequest = 15;
	std::shared_ptr<udp::socket> _socket;
	unsigned char _requestBuffer[maxLengthRequest];
	char _responseBuffer[maxLengthResponse];
	bool _readIndicator;
	std::shared_ptr<boost::asio::deadline_timer> _timer;
	int _countSendRequest;

	void sendBindingRequest();
	void doRead();
	//Action after receiving a response
	void readHandle(const boost::system::error_code& error, size_t bytes);
	//Action after sending a request
	void writeHandle(const boost::system::error_code& error, size_t bytes);
	//Setting of flags after monitoring
	void stunServerisActive();
	//Action after waiting a seconds
	void waitHandle(const boost::system::error_code error);
	//Action after connecting to server
	void connectHandle(const boost::system::error_code error);
};

