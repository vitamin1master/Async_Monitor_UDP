#include "headers/parsing_config.h"
#include <fstream>
#include <iostream>

bool parsing_config::parse(const std::string& config_path)
{
	address_config_file = config_path;
	Json::Value root;
	Json::Reader reader;
	std::ifstream in(address_config_file);

	if (!reader.parse(in, root)) {
		std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": "  << std::endl;
		return false;
	}

	try {
		url_give_servers = root["url_resource"].asString();
		period_sending_request_ms = root["period_sending_request_ms"].asInt();
		max_number_request_sent = root["max_number_request_sent"].asInt();
	}
	catch (const Json::LogicError &er)
	{
		std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": " << er.what() << std::endl;
		return false;
	}
	catch (const std::exception &ec)
	{
		std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": " << ec.what() << std::endl;
		return false;
	}

	if(!check_values())
	{
		return false;
	}

	return true;
}

bool parsing_config::check_values()
{
	if (url_give_servers == "" || period_sending_request_ms == 0 || max_number_request_sent == 0)
	{

		if (url_give_servers == "")
		{
			std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": In the configuration file there is no definition of the url_give_servers or it's value is empty"
					  << std::endl;
		}

		if (period_sending_request_ms == 0)
		{
			std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": In the configuration file there is no definition of the period_sending_request_ms or it's value is 0"
					  << std::endl;
		}

		if (max_number_request_sent == 0)
		{
			std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__<< ": In the configuration file there is no definition of the max_number_request_sent or it's value is 0"
					  << std::endl;
		}

		return false;
	}

	return true;
}
