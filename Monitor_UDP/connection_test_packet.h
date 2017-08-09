#pragma once
#include <boost/asio.hpp>
#include "connection_info.h"
#include "stun_response.h"
#include "stun_request.h"
#include <boost/function.hpp>

class connection_test_packet : public std::enable_shared_from_this<connection_test_packet>
{
public:
	connection_test_packet(connection_info info_, boost::asio::io_service& service, boost::function<void(std::shared_ptr<connection_test_packet>)> func);
	~connection_test_packet();

	static const int _max_length_response = 128;

	bool connect_indicator;
	bool stop_indicator;
	bool read_indicator;

	connection_info info;

	boost::asio::deadline_timer timer;

	int count_send_request;
	stun_response response_header;
	stun_request request_header;
	char response_buffer[_max_length_response];

	boost::asio::ip::udp::endpoint end_point;
	boost::function<void(std::shared_ptr<connection_test_packet>)> connection_stop_handler;
};

