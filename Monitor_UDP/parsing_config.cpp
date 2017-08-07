#include "parsing_config.h"
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include <iostream>
#include <fstream>
//#include <json/value.h>
//#include <json/reader.h>

//public:
parsing_config::parsing_config(std::string path_config) :address_config_file(path_config)
{
	parsing_successful = parsing_config_file();
}

parsing_config::parsing_config(const parsing_config& parsing): parsing_successful(parsing.parsing_successful), address_config_file(parsing.address_config_file), url_give_servers(parsing.url_give_servers),
                                                               address_record_file(parsing.address_record_file), get_command(parsing.get_command)
{
}

parsing_config::~parsing_config()
{
}

//private:
bool parsing_config::parsing_config_file()
{
	Json::Value root;
	Json::Reader reader;
	try 
	{
		std::ifstream in(address_config_file);
		in >> root;
		address_record_file = root["captureRecordFile"].asString();
		url_give_servers = root["urlResource"].asString();
	}
	catch (...)
	{
		//std::cout << "Invalid config file: " << std::endl;
		return false;
	}
	select_get_command();
	return true;
}

void parsing_config::select_get_command()
{
	if (url_give_servers == "")
	{
		return;
	}
	int position = url_give_servers.find('/');
	if (position != std::string::npos)
	{
		get_command = url_give_servers.substr(position);
		url_give_servers.erase(position);
	}
	else
	{
		get_command = "";
	}
}
