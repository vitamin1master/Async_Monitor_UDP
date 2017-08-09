#include "Monitor.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <fstream>
#include "multitask_connection.h"
#include <iostream>
#include <thread>
using boost::asio::ip::tcp;

//public:
Monitor::Monitor(std::shared_ptr<requester_ip_list> &requester) : _requester(requester)
{
	std::shared_ptr<boost::asio::io_service> service(new boost::asio::io_service());
	_io_service.swap(service);
}

Monitor::~Monitor()
{
	_connections_list.clear();
}

void Monitor::start_monitoring()
{
	initialization_components();
	//boost::function<void(std::shared_ptr<Connection>)> func(boost::bind(&Monitor::stop_connection,this,_1));
	//for (auto i = 0; i < _requester->servers_ports_list.size(); ++i)
	//{
	//	
	//	std::shared_ptr<Connection> newConnection_ptr(new Connection(*_io_service, i, _requester->servers_ports_list[i].first, _requester->servers_ports_list[i].second, func));
	//	_connections_list.push_back(newConnection_ptr);
	//	newConnection_ptr->connect();
	//}
	//_io_service->run();

	boost::function<void(std::vector<connection_info> completed_connections_info_list)> ver_res_monitoring_func(boost::bind(&Monitor::verification_result_monitoring, this,_1));
	multitask_connection con(_requester->servers_ports_list, ver_res_monitoring_func);
	con.connect();
}

void Monitor::stop_connection(std::shared_ptr<Connection> connection)
{
	//auto iterator =	std::find(_connections_list.begin(), _connections_list.end(), connection);
	//if (iterator != _connections_list.end())
	//	_connections_list.erase(iterator);
	//connection_info c_info{ connection->server_id, 0, connection->stun_server_is_active, connection->index_Connection, connection->returned_ip_port};
	//_completed_connections_info_list.push_back(c_info);
	//if (!_connections_list.size())
	//{
	//	verification_result_monitoring();
	//}
}

//private:
void Monitor::initialization_components()
{
	_io_service->reset();
	_connections_list.clear();
	_completed_connections_info_list.clear();
}

void Monitor::verification_result_monitoring(std::vector<connection_info> completed_connections_info_list)
{
	//Open json file on write
	std::ofstream json_file(_requester->config.address_record_file);
	Json::Value root;
	try
	{
		//Record server data
		Json::Value servers(Json::arrayValue);
		bool allConnectionsStopped = true;
		for (auto it = completed_connections_info_list.begin(); it != completed_connections_info_list.end(); *it++)
		{
			Json::Value server;
			server["ID"] = it->server_id;
			//server.put("id", it->server_id);
			if (it->stun_server_is_active)
			{
				server["IsActive"] = "Yes";
				//server.put("IsActive", "Yes");
				server["Response"] = it->returned_ip_port;
				//server.put("Response", it->returned_ip_port);
			}
			else
			{
				server["IsActive"] = "No";
				//server.put("IsActive", "No");
			}
			//servers.push_back(std::make_pair("", server));
			servers.append(server);
		}
		//ptree.add_child("servers", servers);
		root["Servers"] = servers;
		json_file << root;
		json_file.close();
		std::cout << "Results recorded" << std::endl;
	}
	catch(...)
	{
		exit(1);
	}
}