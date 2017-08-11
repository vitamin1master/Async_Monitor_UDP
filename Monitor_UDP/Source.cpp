#include "Monitor.h"
#include "parsing_config.h"
#include "requester_ip_list.h"

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	if(argc!=2)
	{
		return 1;
	}

	std::string path_config = argv[1];

	parsing_config config;
	if (!config.parse(argv[1]))
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
