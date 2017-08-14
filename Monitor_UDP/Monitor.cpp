#include "Monitor.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <json/json.h>
#include <json/writer.h>
#include <fstream>
#include "multitask_connection.h"
#include <iostream>
using boost::asio::ip::tcp;

//public:
Monitor::Monitor() :_io_service(std::make_shared<boost::asio::io_service>()), _successful_monitoring_indicator(true)
{
}

Monitor::~Monitor()
{
	_completed_connections_info_list.clear();
}

bool Monitor::start_monitoring(const data_for_monitoring& data_for_monitoring_)
{
	initialization_components(data_for_monitoring_);
	
	multitask_connection mult_con;

	boost::function<void(std::vector<connection_info> completed_connections_info_list)>
		ver_res_monitoring_func(boost::bind(&Monitor::verification_result_monitoring, this, _1));

	if (mult_con.start_checking(data_for_monitoring_.servers_ports_list, ver_res_monitoring_func,
		data_for_monitoring_.period_sending_request_ms, data_for_monitoring_.max_number_request_sent))
	{
		return _successful_monitoring_indicator;
	}
	return false;
}

//private:
void Monitor::initialization_components(const data_for_monitoring& data_for_monitoring_)
{
	_address_record_file = data_for_monitoring_.address_record_file;
	_io_service->reset();
	//_connections_list.clear();
	_completed_connections_info_list.clear();
	_successful_monitoring_indicator = true;
}

void Monitor::verification_result_monitoring(const std::vector<connection_info>& completed_connections_info_list)
{
	//Open json file on write
	std::ofstream json_file(_address_record_file);
	if(!json_file)
	{
		std::cerr << "The address of the record file is incorrect" << std::endl;
		_successful_monitoring_indicator = false;
		return;
	}
	Json::Value root;
	//Record server data
	Json::Value servers(Json::arrayValue);
	for (auto it : completed_connections_info_list)
	{
		Json::Value server;
		server["ID"] = it.server_id;
		if (it.stun_server_is_active)
		{
			server["IsActive"] = "Yes";
			server["Response"] = it.returned_ip_port;
		}
		else
		{
			server["IsActive"] = "No";
		}
		servers.append(server);
	}
	root["Servers"] = servers;
	json_file << root;
	json_file.close();
}
