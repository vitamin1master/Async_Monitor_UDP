#pragma once
#include <string>
#include "json/json.h"

struct parsing_config
{
	std::string address_config_file;
	std::string url_give_servers;
	std::string address_record_file;
	std::string get_command;
	int period_sending_request_ms;
	int max_number_request_sent;

	//Initialization _get_commend

	bool parse(const std::string& config_path);
	bool check_values();
};
