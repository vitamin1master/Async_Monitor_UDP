#include "Monitor.h"
#include "parsing_config.h"
#include "requester_ip_list.h"
#include "program_options_parse.h"
#include <iostream>

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");

	std::string config_path;

	if(!po_parse(argc,argv,config_path))
	{
		return 1;
	}

	parsing_config config;
	if (!config.parse(config_path))
	{
		return 1;
	}

	requester_ip_list requester;
	data_for_monitoring data_for_monitoring_;
	if (!requester.request(config, data_for_monitoring_))
	{
		return 1;
	}

	Monitor monitor;
	if (!monitor.start_monitoring(data_for_monitoring_))
	{
		return 1;
	}

	return 0;
}
