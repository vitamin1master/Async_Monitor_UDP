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
	void StartMonitoring();

private:
	static const int delay = 1000;

	std::string _addressConfigFile;
	std::string _urlGiveServers;
	std::string _addressRecordFile;
	std::string _getCommand;

	std::vector<std::pair<std::string, int>> _servers_ports_list;
	std::shared_ptr<boost::asio::io_service> _io_service;
	std::shared_ptr<tcp::socket> _socket;

	boost::shared_ptr<boost::asio::streambuf> _request;
	std::shared_ptr<boost::asio::streambuf> _response;
	std::shared_ptr<tcp::resolver> _resolver;
	std::vector<std::shared_ptr<Connection>> _connectionsList;
	std::shared_ptr<boost::asio::deadline_timer> _timer;
	bool parsingConfigFile();
	//Fill in the lists of servers and ports from json file
	void listingServersAndPorts(boost::property_tree::ptree const& ptree);
	//Analysis of monitorin data
	void verificationResultMonitoring();

	void stoppedMonitoring(bool success) const;
	void initializationComponents();
	//Action after waiting a seconds
	void waitHandle(const boost::system::error_code error);
	//Fill in the lists of servers and ports received by URL data
	bool getAddressMonitoring();
	void select_getCommand();
};

