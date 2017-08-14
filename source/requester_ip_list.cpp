#include "headers/requester_ip_list.h"
#include <fstream>
#include <iostream>


requester_ip_list::requester_ip_list() : _socket(_io_service), _resolver(_io_service)
{

}

bool requester_ip_list::request(const parsing_config& config, data_for_monitoring& data_for_monitoring_)
{
	if (!send_http_request(config))
	{
		return false;
	}

	if (!receive_http_response(config))
	{
		return false;
	}

	if (!response_analysis(data_for_monitoring_))
	{

		std::cerr << "Invalid download json file: " << std::endl;
		return false;
	}

	data_for_monitoring_.address_record_file = config.address_record_file;
	data_for_monitoring_.max_number_request_sent = config.max_number_request_sent;
	data_for_monitoring_.period_sending_request_ms = config.period_sending_request_ms;

	return true;
}

bool requester_ip_list::listing_servers_and_ports(Json::Value::const_iterator const& branch, data_for_monitoring& data_for_monitoring_)
{
	auto it = branch->begin();
	auto end = branch->end();
	if (it != end)
	{
		std::string server = it->asString();
		*it++;
		if (it == end)
		{
			return false;
		}
		int port = it->asInt();
		if(server==""||port==0)
		{
			return false;
		}
		data_for_monitoring_.servers_ports_list.push_back(std::pair<std::string, int>(server, port));
		return true;
	}
	return false;
}

bool requester_ip_list::send_http_request(const parsing_config& config)
{
	//Create request
	std::iostream request_stream(&_request);
	request_stream << "GET " << config.get_command << " HTTP/1.0\r\n";
	request_stream << "Host: " << config.url_give_servers << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	boost::system::error_code ec;

	//Connect to server
	tcp::resolver::query query(config.url_give_servers, "http");
	tcp::resolver::iterator it = _resolver.resolve(query,ec);
	if (ec)
	{
		std::cerr << "Invalid url_give_servers or problems with connection: " << ec.message() << std::endl;
		return false;
	}

	tcp::resolver::iterator end;

	ec = boost::asio::error::host_not_found;
	while (ec&&it != end)
	{
		_socket.close();
		_socket.connect(*it,ec);
		*it++;
	}
	if (ec)
	{
		std::cerr << "Can't connect by http: " << ec.message() << std::endl;
		return false;
	}

	//Send request
	boost::asio::write(_socket, _request, ec);
	if (ec)
	{
		std::cerr << "Can't send message by http: " << ec.message() << std::endl;
		return false;
	}
	return true;
}

bool requester_ip_list::receive_http_response(const parsing_config& config)
{
	boost::system::error_code ec;

	boost::asio::read_until(_socket, _response, '\n', ec);
	if (ec)
	{
		std::cerr << "Can't read response by http: " << ec.message() << std::endl;
		return false;
	}

	//Get data on response
	std::iostream response_stream(&_response);
	std::string http_version;
	std::string status_message;
	unsigned int status_code;

	response_stream >> http_version;
	response_stream >> status_code;
	std::getline(response_stream, status_message);

	//Check the correctness of the response
	if (!response_stream || http_version.find("HTTP")==std::string::npos)
	{
		std::cerr << "Invalid response" << std::endl;
		return false;
	}

	boost::asio::read_until(_socket, _response, "\r\n\r\n", ec);
	if (ec)
	{
		std::cerr << "Can't read response by http: " << ec.message() << std::endl;
		return false;
	}
	std::string header;

	//Remove superfluous
	while (std::getline(response_stream, header) && header != "\r")
	{
		
	}

	/*Json::Value root;
	std::ostringstream os;
	os << &_response;
	std::string str = os.str();
	std::istringstream is(str);
	is >> root;*/

	std::ifstream read_file("download_file.json");
	Json::Reader reader;
	reader.parse(read_file, _root);

	return true;
}

bool requester_ip_list::response_analysis(data_for_monitoring& data_for_monitoring_)
{
	for (auto it = _root.begin(); it != _root.end(); ++it)
	{
		if (!listing_servers_and_ports(it, data_for_monitoring_))
		{
			return false;
		}
	}
	if (!data_for_monitoring_.servers_ports_list.size())
	{
		return false;
	}
	//std::cout << "Successfully obtained a servers list from " << _url_give_servers << _get_command << std::endl;
	return true;
}
