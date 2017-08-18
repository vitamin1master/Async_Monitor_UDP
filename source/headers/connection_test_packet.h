#pragma once
#include <boost/asio.hpp>
#include "connection_info.h"
#include "stun_request.h"
#include <boost/function.hpp>

class connection_test_packet
{
public:
	connection_test_packet(const connection_info& info_, boost::asio::io_service& service);
	~connection_test_packet();

	static bool is_connect_sougth(const std::shared_ptr<const connection_test_packet>& connection, const boost::asio::ip::udp::endpoint& end_point);

    static const int max_length_response = 128;
	bool stop_indicator;
	bool read_indicator;

	connection_info info;

	boost::asio::deadline_timer timer;

	int count_send_request;
	stun_request request_header;
	char response_buf[max_length_response];

	boost::asio::ip::udp::endpoint end_point;
	boost::asio::ip::udp::endpoint remote_end_point;
};

