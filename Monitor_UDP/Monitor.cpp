#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif

#include "Monitor.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <iostream>
using boost::asio::ip::tcp;

#define binding1(x) boost::bind(&Monitor::x,this)
#define binding2(x,y) boost::bind(&Monitor::x,this,y)
#define binding3(x,y,z) boost::bind(&Monitor::x,this,y,z)

//private:
void Monitor::stoppedMonitoring(bool success) const
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

void Monitor::initializationComponents()
{
	_io_service->stop();
	_io_service->reset();

	for(auto it = _connectionsList.begin(); it != _connectionsList.end(); *it++)
	{
		it->reset();
	}

	_connectionsList.clear();
	_servers_ports_list.clear();

	_resolver.reset();
	std::shared_ptr<tcp::resolver> res_ptr(new tcp::resolver(*_io_service));
	_resolver.swap(res_ptr);

	_request.reset();
	boost::shared_ptr<boost::asio::streambuf> req_ptr(new boost::asio::streambuf());
	_request.swap(req_ptr);

	_response.reset();
	std::shared_ptr<boost::asio::streambuf> resp_ptr(new boost::asio::streambuf());
	_response.swap(resp_ptr);

	_socket.reset();
	std::shared_ptr < tcp::socket > sock_ptr(new tcp::socket(*_io_service));
	_socket.swap(sock_ptr);

	_timer.reset();
	std::shared_ptr < boost::asio::deadline_timer > time_ptr(new boost::asio::deadline_timer(*_io_service));
	_timer.swap(time_ptr);
}

void Monitor::listingServersAndPorts(boost::property_tree::ptree const& ptree)
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

void Monitor::select_getCommand()
{
	if (_urlGiveServers == "")
	{
		return;
	}
	int position = _urlGiveServers.find('/');
	if (position != std::string::npos)
	{
		_getCommand = _urlGiveServers.substr(position);
		_urlGiveServers.erase(position);
	}
	else
	{
		_getCommand = "";
	}
}


bool Monitor::parsingConfigFile()
{
	boost::property_tree::ptree ptree;
	try
	{
		//Read json file
		std::ifstream jsonFile(_addressConfigFile);
		boost::property_tree::read_json(jsonFile, ptree);

		_addressRecordFile = ptree.get<std::string>("captureRecordFile");
		_urlGiveServers = ptree.get<std::string>("urlResource");

		select_getCommand();
		bool flag = getAddressMonitoring();
		return flag;
	}
	catch (std::exception const&e)
	{
		std::cout << "Invalid config" << std::endl;
		return false;
	}
}

void Monitor::verificationResultMonitoring()
{
	//Open json file on write
	std::ofstream jsonFile(_addressRecordFile);
	boost::property_tree::ptree ptree;
	try
	{
		//Record server data
		boost::property_tree::ptree servers;
		bool allConnectionsStopped = true;
		for (auto it = _connectionsList.begin(); it != _connectionsList.end(); *it++)
		{
			boost::property_tree::ptree server;
			server.put("id", it->get()->ServerId);
			if (it->get()->StunServerIsActive)
			{
				server.put("IsActive", "Yes");
				server.put("Response", it->get()->ReturnedIpPort);
			}
			else
			{
				server.put("IsActive", "No");
			}
			std::string serverNumber = "Number " + std::to_string(it->get()->IndexConnection);
			servers.push_back(std::make_pair("", server));
		}
		ptree.add_child("servers", servers);
		boost::property_tree::write_json(jsonFile, ptree);
		std::cout << "Record file at " + _addressRecordFile << std::endl;
		stoppedMonitoring(true);
	}
	catch (std::exception const&e)
	{
		std::cout << "Invalid record file: " << std::endl;
		stoppedMonitoring(false);
	}
}
void Monitor::waitHandle(const boost::system::error_code error)
{
	//Check every seconds on stopped monitoring
	if (error)
	{
		return;
	}
	bool allConnectionsStopped = true;
	for (auto it = _connectionsList.begin(); it != _connectionsList.end(); *it++)
	{
		if (!it->get()->StopIndicator)
		{
			allConnectionsStopped = false;
			break;
		}
	}
	if (!allConnectionsStopped)
	{
		_timer->expires_from_now(boost::posix_time::milliseconds(0));
		_timer->async_wait(binding2(waitHandle, _1));
	}
	else
	{
		verificationResultMonitoring();
	}
}


//public:
Monitor::Monitor(std::string addressConfigFile)
{
	std::shared_ptr<boost::asio::io_service> service(new boost::asio::io_service());
	_io_service.swap(service);

	_addressConfigFile = addressConfigFile;

	std::shared_ptr<tcp::socket> soc_ptr(new tcp::socket(*_io_service));
	_socket.swap(soc_ptr);

	std::shared_ptr<tcp::resolver> res_ptr(new tcp::resolver(*_io_service));
	_resolver.swap(res_ptr);

	std::shared_ptr<boost::asio::streambuf> resp_ptr(new boost::asio::streambuf());
	_response.swap(resp_ptr);

	boost::shared_ptr<boost::asio::streambuf> req_ptr(new boost::asio::streambuf());
	_request.swap(req_ptr);

	std::shared_ptr<boost::asio::deadline_timer> time_ptr(new boost::asio::deadline_timer(*_io_service));
	_timer.swap(time_ptr);

	std::vector<std::shared_ptr<Connection>> con_ptr;
	_connectionsList.swap(con_ptr);
}

bool Monitor::getAddressMonitoring()
{
	//Create request
	if (!_request)
	{
		boost::shared_ptr<boost::asio::streambuf> req_ptr(new boost::asio::streambuf());
		_request.swap(req_ptr);
	}
	std::iostream request_stream(_request.get());
	request_stream << "GET " << _getCommand << " HTTP/1.0\r\n";
	request_stream << "Host: " << _urlGiveServers << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	//Connect to server
	tcp::resolver::query query(_urlGiveServers, "http");
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
	std::ofstream record_file("C:\\FilesRecord\\servers.json",std::ofstream::out);
	record_file << _response;
	record_file.close();
	//Read json file and parse him
	boost::property_tree::ptree ptree;
	std::ifstream read_file("C:\\FilesRecord\\servers.json");
	boost::property_tree::json_parser::read_json(read_file, ptree);
	_servers_ports_list.clear();
	for(auto &iterator: ptree)
	{
		assert(iterator.first.empty());
		listingServersAndPorts(iterator.second);
	}
	if(_servers_ports_list.size())
	{
		std::cout << "Successfully obtained a servers list from " << _urlGiveServers << _getCommand << std::endl;
		return true;
	}
	return false;
}

void Monitor::StartMonitoring()
{
	initializationComponents();
	if (!parsingConfigFile())
	{
		stoppedMonitoring(false);
		return;
	}
	std::cout << "Start monitoring" << std::endl;
	if (!_servers_ports_list.size())
	{
		std::cout << "Received an empty list of servers" << std::endl;
		stoppedMonitoring(false);
		return;
	}
	for (int i = 0; i < _servers_ports_list.size(); i++)
	{
		std::shared_ptr<Connection> newConnection_ptr(new Connection(*_io_service, i, _servers_ports_list[i].first, _servers_ports_list[i].second));
		_connectionsList.push_back(newConnection_ptr);
		newConnection_ptr->Connect();
	}
	_timer->expires_from_now(boost::posix_time::milliseconds(0));
	_timer->async_wait(binding2(waitHandle, _1));
	_io_service->run();
	_io_service->stop();
}
Monitor::~Monitor()
{
	for (auto it = _connectionsList.begin(); it != _connectionsList.end(); *it++)
	{
		it->reset();
		it->reset();
	}

	_connectionsList.clear();
	_servers_ports_list.clear();
	_resolver.reset();
	_socket.reset();
	_timer.reset();
	_request.reset();
	_response.reset();
	_io_service.reset();
}