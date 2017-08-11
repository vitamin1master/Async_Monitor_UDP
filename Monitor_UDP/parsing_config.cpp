#include "parsing_config.h"
#include <json/value.h>
#include <json/reader.h>
#include <fstream>

bool parsing_config::parse(std::string config_path)
{
	address_config_file = config_path;
	Json::Value root;
	Json::Reader reader;
	std::ifstream in(address_config_file);

	if (!reader.parse(in, root))
	{
		return false;
	}

	address_record_file = root["captureRecordFile"].asString();
	url_give_servers = root["urlResource"].asString();

	if (address_record_file == "" || url_give_servers == "")
	{
		return false;
	}

	//Select get_command
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

	return true;
}
