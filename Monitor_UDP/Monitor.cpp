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
void Monitor::stoppedMonitoring(bool success)
{
	if (success)
	{
		std::cout << "Monitoring was success" << std::endl;
	}
	else
	{
		std::cout << "Monitoring was unsuccessful" << std::endl;
	}
	bool allConnectionsStopped = true;
	for (auto it = _connectionsList.begin(); it != _connectionsList.end(); *it++)
	{
		it->reset();
	}

	_connectionsList.clear();

	_serversList.clear();
	_portsList.clear();

	_resolver.reset();
	std::shared_ptr<tcp::resolver> res_ptr(new tcp::resolver(_io_service));
	_resolver.swap(res_ptr);

	_request.reset();
	boost::shared_ptr<boost::asio::streambuf> req_ptr(new boost::asio::streambuf());
	_request.swap(req_ptr);

	_response.reset();
	std::shared_ptr<boost::asio::streambuf> resp_ptr(new boost::asio::streambuf());
	_response.swap(resp_ptr);

	_socket.reset();
	std::shared_ptr < tcp::socket > sock_ptr(new tcp::socket(_io_service));
	_socket.swap(sock_ptr);

	_timer.reset();
	std::shared_ptr < boost::asio::deadline_timer > time_ptr(new boost::asio::deadline_timer(_io_service));
	_timer.swap(time_ptr);
}

void Monitor::listingConfigFile(boost::property_tree::ptree const& ptree)
{
	boost::property_tree::ptree::const_iterator it = ptree.begin();
	boost::property_tree::ptree::const_iterator end = ptree.end();
	if (it != end)
	{
		_serversList.push_back(it->second.get_value<std::string>());
	}
	*it++;
	if (it != end)
	{
		_portsList.push_back(it->second.get_value<int>());
	}
}

void Monitor::select_getCommand()
{
	if (_urlGiveServers == "")
		return;
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
		getAddressMonitoring();
	}
	catch (std::exception const&e)
	{
		std::cout << "Parsing fail" << e.what();
		return false;
	}
	return true;
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
		std::cout << "Invalid record file: " << e.what();
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
	_addressConfigFile = addressConfigFile;
	std::shared_ptr<tcp::socket> soc_ptr(new tcp::socket(_io_service));
	_socket.swap(soc_ptr);

	std::shared_ptr<tcp::resolver> res_ptr(new tcp::resolver(_io_service));
	_resolver.swap(res_ptr);

	std::shared_ptr<boost::asio::streambuf> resp_ptr(new boost::asio::streambuf());
	_response.swap(resp_ptr);

	boost::shared_ptr<boost::asio::streambuf> req_ptr(new boost::asio::streambuf());
	_request.swap(req_ptr);

	std::shared_ptr<boost::asio::deadline_timer> time_ptr(new boost::asio::deadline_timer(_io_service));
	_timer.swap(time_ptr);

	std::vector<std::shared_ptr<Connection>> con_ptr;
	_connectionsList.swap(con_ptr);
}

void Monitor::getAddressMonitoring()
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
	tcp::resolver::iterator it = _resolver->resolve(query);
	tcp::resolver::iterator end;
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error&&it != end)
	{
		_socket->close();
		_socket->connect(*it, error);
		*it++;
	}
	if (error)
		return;

	//Send request
	try
	{
		boost::asio::write(*_socket.get(), *_request.get());
	}
	catch (std::exception const&e)
	{
		std::cout << e.what();
		return;
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
		return;
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
	_serversList.clear();
	for(auto &iterator: ptree)
	{
		assert(iterator.first.empty());
		listingConfigFile(iterator.second);
	}
	if(_serversList.size())
	{
		std::cout << "Successfully obtained a servers list from " << _urlGiveServers << _getCommand << std::endl;
	}
}

void Monitor::StartMonitoring()
{
	if (!parsingConfigFile())
	{
		std::cout << "Invalid config" << std::endl;
		stoppedMonitoring(false);
		return;
	}
	std::cout << "Start monitoring" << std::endl;
	if (!_serversList.size())
	{
		std::cout << "Received an empty list of servers" << std::endl;
		stoppedMonitoring(false);
		return;
	}
	for (int i = 0; i < _serversList.size(); i++)
	{
		std::shared_ptr<Connection> newConnection_ptr(new Connection(_io_service, i, _serversList[i], _portsList[i]));
		_connectionsList.push_back(newConnection_ptr);
		newConnection_ptr->Connect();
	}
	_timer->expires_from_now(boost::posix_time::milliseconds(0));
	_timer->async_wait(binding2(waitHandle, _1));
	_io_service.run();
	std::shared_ptr<boost::asio::io_service::work> work_ptr(new boost::asio::io_service::work(_io_service));
	_work.swap(work_ptr);
}
Monitor::~Monitor()
{
	for (auto it = _connectionsList.begin(); it != _connectionsList.end(); *it++)
	{
		it->reset();
		it->reset();
	}

	_connectionsList.clear();
	_resolver.reset();
	_socket.reset();
	_timer.reset();
	_request.reset();
	_response.reset();
	_work.reset();
}