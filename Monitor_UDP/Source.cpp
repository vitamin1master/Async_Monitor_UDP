#include <boost/asio.hpp>
#include <iostream>
#include "Monitor.h"

void main()
{
	std::cout << "Hello world" << std::endl;
	Monitor monitor("config.json");
	monitor.StartMonitoring();
	getchar();
}
