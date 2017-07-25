#pragma once

#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "Connection.h"
using boost::asio::ip::tcp;


class Monitor
{
public:
	Monitor(std::string addressConfigFile);
	~Monitor();
	//void MonitoringOneOfTheServersIsComplete(boost::shared_ptr<Connection> connection);
	void StartMonitoring();
	void StartMonitoring1();

private:
	static const int delay = 1000;

	std::string _addressConfigFile;
	std::string _urlGiveServers;
	std::string _addressRecordFile;
	std::string _getCommand;

	std::vector<std::string> _serversList;
	std::vector<int> _portsList;
	boost::asio::io_service _io_service;
	std::shared_ptr<tcp::socket> _socket;

	boost::shared_ptr<boost::asio::streambuf> _request;
	std::shared_ptr<boost::asio::streambuf> _response;
	std::shared_ptr<tcp::resolver> _resolver;
	std::shared_ptr<boost::asio::io_service::work> _work;
	std::vector<std::shared_ptr<Connection>> _connectionsList;
	std::shared_ptr<boost::asio::deadline_timer> _timer;

	bool parsingConfigFile();
	void listingConfigFile(boost::property_tree::ptree const& ptree);

	void verificationResultMonitoring();

	void stoppedMonitoring(bool success);

	void waitHandle(const boost::system::error_code error);

	void getAddressMonitoring();

	void select_getCommand();
};

