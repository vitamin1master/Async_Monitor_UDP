#pragma once

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <memory>
#include "stun_response.h"
#include "stun_request.h"
#include "connection_info.h"
#include "connection_test_packet.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class multitask_connection
{
public:
	multitask_connection(std::vector<std::pair<std::string, int>> servers_ports_list, boost::function<void(std::vector<connection_info> completed_connections_info_list)> func);
	~multitask_connection();

	
	void connect();

	std::vector<std::shared_ptr<connection_test_packet>> connections_list;
private:
	void connect_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error);
	void send_binding_request(std::shared_ptr<connection_test_packet>& connection);
	void start_connection(std::shared_ptr<connection_test_packet>& connection);
	void wait_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error);
	void write_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes);
	void stop_connection(std::shared_ptr<connection_test_packet>& connection);
	void read_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes);
	void do_read(std::shared_ptr<connection_test_packet>& connection);
	void server_is_active(std::shared_ptr<connection_test_packet>& connection);
	std::vector<connection_info> completed_connections_info_list;

	static const int _delay = 500;
	static const int _max_count_send_request = 15;
	static const int _interval = 30;
	boost::asio::io_service _io_service;
	udp::socket _socket;
	boost::function<void(std::vector<connection_info> completed_connections_info_list)> all_connections_stopped_handle;
};

