#pragma once
#include <string>
#include <vector>

struct data_for_monitoring
{
	std::vector<std::pair<std::string, int>> servers_ports_list;
	std::string address_record_file;
};
