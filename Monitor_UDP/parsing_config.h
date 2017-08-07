#pragma once
#include <string>
class parsing_config
{
public:
	parsing_config(std::string path_config);
	parsing_config(const parsing_config& parsing);
	~parsing_config();
	
	bool parsing_successful;

	std::string address_config_file;
	std::string url_give_servers;
	std::string address_record_file;
	std::string get_command;

private:
	bool parsing_config_file();
	//Initialization _get_commend
	void select_get_command();
};