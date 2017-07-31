#pragma once

#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif

#include <boost/asio.hpp>
#include "Declare.h"
#include "Monitor.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	Connection(boost::asio::io_service& io_service, int indexConnection, std::string serverId, int serverPort, Monitor* monitor);
	~Connection();

	void Connect();
	std::shared_ptr< udp::socket> Socket();

	int index_Connection;
	bool stun_server_is_active;
	bool stop_indicator;
	std::string server_id;
	int server_port;
	std::string returned_ip_port;
	
private:
	void start_connection();
	void send_binding_request();
	void do_read();
	//Action after receiving a response
	void read_handle(const boost::system::error_code& error, size_t bytes);
	//Action after sending a request
	void write_handle(const boost::system::error_code& error, size_t bytes);
	//Setting of flags after monitoring
	void stun_server_isActive();
	//Action after waiting a seconds
	void wait_handle(const boost::system::error_code error);
	//Action after connecting to server
	void connect_handle(const boost::system::error_code error);

	static const int _max_length_response = 128;
	static const int _max_length_request = 20;
	static const int _delay = 1000;
	static const int _max_count_send_request = 15;
	std::shared_ptr<udp::socket> _socket;
	unsigned char _request_buffer[_max_length_request];
	char _response_buffer[_max_length_response];
	bool _read_indicator;
	std::shared_ptr<boost::asio::deadline_timer> _timer;
	int _count_send_request;
	Monitor* _monitor;
};

