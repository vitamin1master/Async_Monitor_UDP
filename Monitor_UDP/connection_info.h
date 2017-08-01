#pragma once
#include <string>

struct connection_info
{
	std::string server_id;
	bool stun_server_is_active;
	std::string returned_ip_port;
	int index_Connection;
};
