#include <boost/asio.hpp>
#include <iostream>
#include "Monitor.h"

void main()
{
	Monitor monitor("config.json");
	monitor.StartMonitoring();
	getchar();
}
