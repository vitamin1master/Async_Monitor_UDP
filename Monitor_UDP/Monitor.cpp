#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif

#include "Monitor.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
using boost::asio::ip::tcp;

#define binding1(x) boost::bind(&Monitor::x,this)
#define binding2(x,y) boost::bind(&Monitor::x,this,y)
#define binding3(x,y,z) boost::bind(&Monitor::x,this,y,z)

//public:
Monitor::Monitor(std::string address_config_file)
{
	std::shared_ptr<boost::asio::io_service> service(new boost::asio::io_service());
	_io_service.swap(service);

	_address_config_file = address_config_file;

	_socket.reset(new tcp::socket(*_io_service));

	//tcp::resolver new_resolver(*_io_service);
	//_resolver=std::make_shared<tcp::resolver>(new_resolver);
	_resolver.reset(new tcp::resolver(*_io_service));

	_response.reset(new boost::asio::streambuf());

	_request.reset(new boost::asio::streambuf());
}

Monitor::~Monitor()
{
	_connections_list.clear();
	_servers_ports_list.clear();
}

void Monitor::start_monitoring()
{
	initialization_components();
	if (!parsing_config_file())
	{
		stop_monitoring(false);
		return;
	}
	std::cout << "Start monitoring" << std::endl;
	if (!_servers_ports_list.size())
	{
		std::cout << "Received an empty list of servers" << std::endl;
		stop_monitoring(false);
		return;
	}
	for (int i = 0; i < _servers_ports_list.size(); i++)
	{
		boost::function<void(std::shared_ptr<Connection>)> func(boost::bind(&Monitor::stop_connection,this,_1));
		std::shared_ptr<Connection> newConnection_ptr(new Connection(*_io_service, i, _servers_ports_list[i].first, _servers_ports_list[i].second, func));
		_connections_list.push_back(newConnection_ptr);
		newConnection_ptr->connect();
	}
	_io_service->run();
}

void Monitor::stop_connection(std::shared_ptr<Connection> connection)
{
	auto iterator =	std::find(_connections_list.begin(), _connections_list.end(), connection);
	if (iterator != _connections_list.end())
		_connections_list.erase(iterator);
	connection_info c_info{ connection->server_id, connection->stun_server_is_active, connection->returned_ip_port, connection->index_Connection };
	_completed_connections_info_list.push_back(c_info);
	if (!_connections_list.size())
	{
		verification_result_monitoring();
	}
}

//private:
void Monitor::stop_monitoring(bool success) const
{
	if (success)
	{
		std::cout << "Monitoring was success" << std::endl;
	}
	else
	{
		std::cout << "Monitoring was unsuccessful" << std::endl;
	}
}

void Monitor::initialization_components()
{
	_io_service->reset();

	_connections_list.clear();
	_completed_connections_info_list.clear();
	_servers_ports_list.clear();


	_resolver.reset(new tcp::resolver(*_io_service));

	_request.reset(new boost::asio::streambuf());

	_response.reset(new boost::asio::streambuf());

	_socket.reset(new tcp::socket(*_io_service));
}

void Monitor::listing_servers_and_ports(boost::property_tree::ptree const& ptree)
{
	auto it = ptree.begin();
	auto end = ptree.end();
	if (it != end)
	{
		std::string server = it->second.get_value<std::string>();
		*it++;
		int port = it->second.get_value<int>();
		_servers_ports_list.push_back(std::pair<std::string, int>(server,port));
	}
}

void Monitor::select_get_command()
{
	if (_url_give_servers == "")
	{
		return;
	}
	int position = _url_give_servers.find('/');
	if (position != std::string::npos)
	{
		_get_command = _url_give_servers.substr(position);
		_url_give_servers.erase(position);
	}
	else
	{
		_get_command = "";
	}
}


bool Monitor::parsing_config_file()
{
	boost::property_tree::ptree ptree;
	try
	{
		//Read json file
		std::ifstream jsonFile(_address_config_file);
		boost::property_tree::read_json(jsonFile, ptree);

		_address_record_file = ptree.get<std::string>("captureRecordFile");
		_url_give_servers = ptree.get<std::string>("urlResource");

		select_get_command();
		bool flag = get_address_monitoring();
		return flag;
	}
	catch (std::exception const&e)
	{
		std::cout << "Invalid config" << std::endl;
		return false;
	}
}

void Monitor::verification_result_monitoring()
{
	//Open json file on write
	std::ofstream jsonFile(_address_record_file);
	boost::property_tree::ptree ptree;
	try
	{
		//Record server data
		boost::property_tree::ptree servers;
		bool allConnectionsStopped = true;
		for (auto it = _completed_connections_info_list.begin(); it != _completed_connections_info_list.end(); *it++)
		{
			boost::property_tree::ptree server;
			server.put("id", it->server_id);
			if (it->stun_server_is_active)
			{
				server.put("IsActive", "Yes");
				server.put("Response", it->returned_ip_port);
			}
			else
			{
				server.put("IsActive", "No");
			}
			std::string serverNumber = "Number " + std::to_string(it->index_Connection);
			servers.push_back(std::make_pair("", server));
		}
		ptree.add_child("servers", servers);
		boost::property_tree::write_json(jsonFile, ptree);
		std::cout << "Record file at " + _address_record_file << std::endl;
		stop_monitoring(true);
	}
	catch (std::exception const&e)
	{
		std::cout << "Invalid record file: " << std::endl;
		stop_monitoring(false);
	}
}

bool Monitor::get_address_monitoring()
{
	//Create request
	if (!_request)
	{
		_request.reset(new boost::asio::streambuf());
	}
	std::iostream request_stream(_request.get());
	request_stream << "GET " << _get_command << " HTTP/1.0\r\n";
	request_stream << "Host: " << _url_give_servers << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	//Connect to server
	tcp::resolver::query query(_url_give_servers, "http");
	try 
	{
		tcp::resolver::iterator it = _resolver->resolve(query);
		tcp::resolver::iterator end;
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error&&it != end)
		{
			_socket->close();
			_socket->connect(*it);
			*it++;
		}
	}
	catch (std::exception const&e)
	{
		std::cout << "Invalid URL or connections problems occurred" << std::endl;
		return false;
	}

	//Send request
	try
	{
		boost::asio::write(*_socket.get(), *_request.get());
	}
	catch (std::exception const&e)
	{
		std::cout << "Error sending request" << std::endl;
		return false;
	}
	boost::asio::read_until(*_socket, *_response, '\n');

	//get data on response
	std::iostream response_stream(_response.get());
	std::string http_version;
	std::string status_message;
	unsigned int status_code;

	response_stream >> http_version;
	response_stream >> status_code;
	std::getline(response_stream, status_message);
	//Check the correctness of the response
	if (!response_stream&&http_version.substr(0, 5) != "HTTP")
	{
		std::cout << "Invalid response" << std::endl;
		return false;
	}
	boost::asio::read_until(*_socket, *_response, '\r\n\r\n');
	std::string header;

	//Remove superfluous
	while (std::getline(response_stream, header) && header != "\r");
	//Write the main part of message in a file
	std::ofstream record_file("servers.json",std::ofstream::out);
	record_file << _response;
	record_file.close();
	//Read json file and parse him
	boost::property_tree::ptree ptree;
	std::ifstream read_file("servers.json");
	boost::property_tree::json_parser::read_json(read_file, ptree);
	_servers_ports_list.clear();
	for(auto &iterator: ptree)
	{
		assert(iterator.first.empty());
		listing_servers_and_ports(iterator.second);
	}
	if(_servers_ports_list.size())
	{
		std::cout << "Successfully obtained a servers list from " << _url_give_servers << _get_command << std::endl;
		return true;
	}
	return false;
}

