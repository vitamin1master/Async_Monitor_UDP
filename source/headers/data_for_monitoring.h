#pragma once
#include <string>
#include <vector>

struct data_for_monitoring
{
	std::vector<std::pair<std::string, int>> servers_ports_list;
	int period_sending_request_ms;
	int max_number_request_sent;
};
