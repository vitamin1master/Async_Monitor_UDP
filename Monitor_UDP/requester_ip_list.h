#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "parsing_config.h"
#include <json/value.h>

using boost::asio::ip::tcp;
class requester_ip_list
{
public:
	requester_ip_list(const parsing_config& parsing);
	~requester_ip_list();
	
	bool request_successful;
	std::vector<std::pair<std::string, int>> servers_ports_list;
	parsing_config config;
private:
	boost::asio::io_service _io_service;
	tcp::socket _socket;

	boost::asio::streambuf _request;
	boost::asio::streambuf _response;
	tcp::resolver _resolver;

	

	//Fill in the lists of servers and ports from json file
	void listing_servers_and_ports(Json::Value::const_iterator const& branch);
	bool get_addresses_monitoring();
};

