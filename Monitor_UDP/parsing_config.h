#pragma once
#include <string>

struct parsing_config
{
	std::string address_config_file;
	std::string url_give_servers;
	std::string address_record_file;
	std::string get_command;

	//Initialization _get_commend

	bool parse(std::string config_path);
};
