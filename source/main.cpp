#include "headers/monitor.h"
#include "headers/program_options_parse.h"
#include "headers/requester.h"
#include <iostream>

int main(int argc, char* argv[])
{
	std::string config_path;
    std::string record_path;

	if(!po_parse(argc,argv,config_path,record_path))
	{
		return 1;
	}

	parsing_config config;
	if (!config.parse(config_path))
	{
		std::cerr<<"Could not complete the parsing config file"<<std::endl;
		return 1;
	}

	//requester_ip_list requester;
	data_for_monitoring data_for_monitoring_;

    requester requester;
    if(!requester.request(config,data_for_monitoring_))
    {
        std::cerr<<"Could not complete the requesting URL"<<std::endl;
        return 1;
    }

	monitor monitor;
	if (!monitor.start_monitoring(data_for_monitoring_,record_path))
	{
		std::cerr<<"Could not complete the monitoring"<<std::endl;
		return 1;
	}

	return 0;
}