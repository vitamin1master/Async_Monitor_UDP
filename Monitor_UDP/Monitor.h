#pragma once
#include <boost/asio.hpp>
#include "connection_info.h"
#include "requester_ip_list.h"
using boost::asio::ip::tcp;

class Monitor 
{
public:
	//Analysis of monitorin data
	Monitor();
	~Monitor();
	bool start_monitoring(const data_for_monitoring& data_for_monitoring_);

private:
	void verification_result_monitoring(const std::vector<connection_info>& completed_connections_info_list);

	void initialization_components(const data_for_monitoring& data_for_monitoring_);

	std::shared_ptr<boost::asio::io_service> _io_service;
	std::vector<connection_info> _completed_connections_info_list;
	std::string _address_record_file;
	bool _successful_monitoring_indicator;
};

