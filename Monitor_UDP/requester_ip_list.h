#pragma once
#include <boost/asio.hpp>
#include "parsing_config.h"
#include <json/value.h>
#include "data_for_monitoring.h"

using boost::asio::ip::tcp;
class requester_ip_list
{
public:
	requester_ip_list();

	bool request(const parsing_config& config, data_for_monitoring& data_for_monitoring_);

private:

	boost::asio::io_service _io_service;
	tcp::socket _socket;

	boost::asio::streambuf _request;
	boost::asio::streambuf _response;
	tcp::resolver _resolver;

	Json::Value _root;

	//Fill in the lists of servers and ports from json file
	bool listing_servers_and_ports(Json::Value::const_iterator const& branch, data_for_monitoring& data_for_monitoring_);
	bool send_http_request(const parsing_config& config);
	bool receive_http_response(const parsing_config& config);
	bool response_analysis(data_for_monitoring& data_for_monitoring_);
};

