#include "Monitor.h"
#include "parsing_config.h"
#include "requester_ip_list.h"

int main(int argc, char* argv[])
{
	if(argc!=2)
	{
		exit(1);
	}
	std::string path_config = argv[1];
	parsing_config config(path_config);
	if (!config.parsing_successful)
	{
		exit(1);
	}
	std::shared_ptr<requester_ip_list> requester;
	requester.reset(new requester_ip_list(config));
	if (!requester->request_successful)
	{
		exit(1);
	}
	Monitor monitor(requester);
	try 
	{
		monitor.start_monitoring();
	}
	catch (...)
	{
		exit(1);
	}
	return 0;
}
