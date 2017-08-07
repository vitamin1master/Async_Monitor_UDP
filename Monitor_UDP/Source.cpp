#include "Monitor.h"
#include "parsing_config.h"
#include <json/reader.h>
#include <json/value.h>
#include "requester_ip_list.h"

int main()
{
	parsing_config config("config.json");
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
	catch(std::system_error e)
	{
		exit(1);
	}
	return 0;
}
