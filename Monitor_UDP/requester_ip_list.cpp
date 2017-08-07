#include "requester_ip_list.h"
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>


requester_ip_list::requester_ip_list(const parsing_config& parsing) :config(parsing),_socket(_io_service), _resolver(_io_service)
{
	if (!config.parsing_successful)
	{
		request_successful = config.parsing_successful;
		exit(1);
	}
	try
	{
		request_successful = get_addresses_monitoring();
	}
	catch(...)
	{
		exit(1);
	}
}

requester_ip_list::~requester_ip_list()
{
}

void requester_ip_list::listing_servers_and_ports(Json::Value::const_iterator const& branch)
{
	auto it = branch->begin();
	auto end = branch->end();
	if (it != end)
	{
		std::string server = it->asString();
		++it;
		int port = it->asInt();
		servers_ports_list.push_back(std::pair<std::string, int>(server, port));
	}
}

bool requester_ip_list::get_addresses_monitoring()
{
	//Create request
	std::iostream request_stream(&_request);
	request_stream << "GET " << config.get_command << " HTTP/1.0\r\n";
	request_stream << "Host: " << config.url_give_servers << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	//Connect to server
	tcp::resolver::query query(config.url_give_servers, "http");
	try
	{
		tcp::resolver::iterator it = _resolver.resolve(query);
		tcp::resolver::iterator end;
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error&&it != end)
		{
			_socket.close();
			_socket.connect(*it);
			*it++;
		}
	}
	catch (...)
	{
		//std::cout << "Invalid URL or connections problems occurred" << std::endl;
		return false;
	}

	//Send request
	try
	{
		boost::asio::write(_socket, _request);
	}
	catch (...)
	{
		//std::cout << "Error sending request" << std::endl;
		return false;
	}
	boost::asio::read_until(_socket, _response, '\n');

	//get data on response
	std::iostream response_stream(&_response);
	std::string http_version;
	std::string status_message;
	unsigned int status_code;

	response_stream >> http_version;
	response_stream >> status_code;
	std::getline(response_stream, status_message);
	//Check the correctness of the response
	if (!response_stream&&http_version.substr(0, 5) != "HTTP")
	{
		//std::cout << "Invalid response" << std::endl;
		return false;
	}
	boost::asio::read_until(_socket, _response, "\r\n\r\n");
	std::string header;

	//Remove superfluous
	while (std::getline(response_stream, header) && header != "\r")
	{
		
	}
	//Write the main part of message in a file
	std::ofstream record_file("download_file.json", std::ofstream::out);
	record_file << &_response;
	record_file.close();
	//Read json file and parse him
	Json::Value root;
	std::ifstream read_file("download_file.json");
	read_file >> root;
	servers_ports_list.clear();
	for (auto it = root.begin(); it != root.end(); ++it)
	{
		listing_servers_and_ports(it);
	}
	if (servers_ports_list.size())
	{
		//std::cout << "Successfully obtained a servers list from " << _url_give_servers << _get_command << std::endl;
		return true;
	}
	return false;
}
