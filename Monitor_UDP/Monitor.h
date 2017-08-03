#pragma once


#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "Connection.h"
#include "connection_info.h"
#include <memory.h>
#include <stdio.h>
using boost::asio::ip::tcp;

class Monitor 
{
public:
	Monitor(std::string address_config_file);
	~Monitor();
	void start_monitoring();
private:

	std::string _address_config_file;
	std::string _url_give_servers;
	std::string _address_record_file;
	std::string _get_command;

	std::vector<std::pair<std::string, int>> _servers_ports_list;
	std::shared_ptr<boost::asio::io_service> _io_service;
	std::shared_ptr<tcp::socket> _socket;

	boost::shared_ptr<boost::asio::streambuf> _request;
	std::shared_ptr<boost::asio::streambuf> _response;
	std::shared_ptr<tcp::resolver> _resolver;
	std::vector<std::shared_ptr<Connection>> _connections_list;
	std::vector<connection_info> _completed_connections_info_list;

	void stop_connection(std::shared_ptr<Connection> connection);
	bool parsing_config_file();
	//Fill in the lists of servers and ports from json file
	void listing_servers_and_ports(boost::property_tree::ptree const& ptree);
	//Analysis of monitorin data
	void verification_result_monitoring();

	void stop_monitoring(bool success) const;
	void initialization_components();
	//Fill in the lists of servers and ports received by URL data
	bool get_address_monitoring();
	void select_get_command();
};

