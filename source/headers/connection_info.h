#pragma once
#include <string>
struct connection_info
{
	std::string server_id;
	int server_port;
	bool stun_server_is_active;
	int index_Connection;
	std::string returned_ip_port;
};