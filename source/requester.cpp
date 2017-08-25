#include "headers/requester.h"
#include <iostream>
#include <cstring>

requester::requester()
{
    _curl=curl_easy_init();
}

int requester::read_handle(char *data, size_t size, size_t nmemb, std::string* buffer) {
    int res;
    res=size * nmemb;
    buffer->append(data,res);
    return res;
}

bool requester::get_page(const parsing_config &config)
{
    curl_easy_setopt(_curl, CURLOPT_URL, config.url_give_servers.c_str());
    curl_easy_setopt(_curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, requester::read_handle);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_buffer);
    _result=curl_easy_perform(_curl);

    if(_result==CURLE_OK)
    {
        std::istringstream is(_buffer);

        std::string http_version;
        std::string status_message;
        unsigned int status_code;

        is >> http_version;
        is >> status_code;
        std::getline(is, status_message);

        //Check the correctness of the response
        if (!is || http_version.find("HTTP") !=0) {
            std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": Invalid response" << std::endl;
            return false;
        }

        std::string header;

        //Remove superfluous
        while (std::getline(is, header) && header != "\r") {

        }

        try
        {
            is >> _root;
        }
        catch (const Json::RuntimeError& er)
        {
            std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": Invalid download page"<<std::endl;
            return false;
        }
        catch (const std::exception& ex)
        {
            std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": Invalid download page" << std::endl;
            return false;
        }
        return true;
    }

    std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": Could't get page"<<std::endl;
    return false;
}

bool requester::request(const parsing_config &config, data_for_monitoring &data_for_monitoring_)
{
    if(_curl != nullptr) {
        if (!get_page(config)) {
            return false;
        }
        if (!response_analysis(data_for_monitoring_))
        {
            return false;
        }
    }
    else
    {
        std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ <<": Could't open cURL"<<std::endl;
        return false;
    }

    data_for_monitoring_.max_number_request_sent = config.max_number_request_sent;
    data_for_monitoring_.period_sending_request_ms = config.period_sending_request_ms;

    return true;
}

bool requester::response_analysis(data_for_monitoring &data_for_monitoring_)
{
    if(!_root.isArray())
    {
        std::cerr<< __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": The downloaded file does not contain an array of servers"<<std::endl;
        return false;
    }

    for(auto it:_root)
    {
        if(!it.isMember("ip") || !it.isMember("port"))
        {
            return false;
        }
        try
        {
            data_for_monitoring_.servers_ports_list.emplace_back(it["ip"].asString(), it["port"].asInt());
        }
        catch(const Json::LogicError& er)
        {
            std::cerr << __PRETTY_FUNCTION__ << ":" <<  __LINE__ << ": " <<er.what()<<std::endl;
            return false;
        }
        catch(const std::exception& ex)
        {
            return false;
        }
    }

    return true;
}

requester::~requester()
{
    curl_easy_cleanup(_curl);
}
