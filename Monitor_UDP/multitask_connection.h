#pragma once

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <vector>
#include "connection_info.h"
#include "connection_test_packet.h"
#include "stun_response.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class multitask_connection
{
public:
	multitask_connection();
	~multitask_connection();

	bool start_checking(const std::vector<std::pair<std::string, int>>& servers_ports_list, const boost::function<void(std::vector<connection_info> completed_connections_info_list)>& func);

	static const int delay = 500;
	static const int max_count_send_request = 30;
	static const int max_length_response = 128;
	static const int interval = 50;

private:
	void connect_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error);
	void wait_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code error);
	void write_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes);
	void read_handle(std::shared_ptr<connection_test_packet>& connection, const boost::system::error_code& error, size_t bytes);

	bool check_response(stun_response& response_struct, const std::shared_ptr<connection_test_packet>& connection, const size_t& bytes);
	
	void send_binding_request(std::shared_ptr<connection_test_packet>& connection);
	void do_read(std::shared_ptr<connection_test_packet>& connection);
	
	void start_connection(std::shared_ptr<connection_test_packet>& connection);
	void stop_connection(std::shared_ptr<connection_test_packet>& connection);
	void server_is_active(std::shared_ptr<connection_test_packet>& connection);

	std::vector<connection_info> _completed_connections_info_list;
	std::vector<std::shared_ptr<connection_test_packet>> _connections_list;
	
	boost::asio::io_service _io_service;
	udp::socket _socket;
	boost::function<void(std::vector<connection_info>)> _all_connections_stopped_handle;
};

