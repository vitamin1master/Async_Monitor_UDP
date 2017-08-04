#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include "Monitor.h"

int main()
{
	Monitor monitor("config.json");
	monitor.start_monitoring();
	return 0;
}
