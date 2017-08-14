#pragma once
#include <boost/asio.hpp>
#include "connection_info.h"
#include "stun_request.h"
#include <boost/function.hpp>

class connection_test_packet
{
public:
	connection_test_packet(const connection_info& info_, boost::asio::io_service& service, const boost::function<void(const std::shared_ptr<const connection_test_packet>&)>& func);
	~connection_test_packet();

	static bool isConnectSougth(const std::shared_ptr<const connection_test_packet>& connection, const boost::asio::ip::udp::endpoint& end_point);

	bool stop_indicator;
	bool read_indicator;

	connection_info info;

	boost::asio::deadline_timer timer;

	int count_send_request;
	stun_request request_header;
	char* response_buf;

	boost::asio::ip::udp::endpoint end_point;
	boost::asio::ip::udp::endpoint remote_end_point;
	boost::function<void(const std::shared_ptr<const connection_test_packet>&)> connection_stop_handler;
};

