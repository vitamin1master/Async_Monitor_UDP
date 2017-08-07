#pragma once


#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include "Connection.h"
#include "connection_info.h"
#include "requester_ip_list.h"
#include <memory.h>
using boost::asio::ip::tcp;

class Monitor 
{
public:
	Monitor(std::shared_ptr<requester_ip_list> &requester);
	~Monitor();
	void start_monitoring();

private:
	std::shared_ptr<boost::asio::io_service> _io_service;
	std::vector<std::shared_ptr<Connection>> _connections_list;
	std::vector<connection_info> _completed_connections_info_list;
	std::shared_ptr<requester_ip_list> _requester;

	void stop_connection(std::shared_ptr<Connection> connection);
	//Analysis of monitorin data
	void verification_result_monitoring();

	void initialization_components();
};

