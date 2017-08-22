#pragma once

#include <curl/curl.h>
#include "parsing_config.h"
#include "data_for_monitoring.h"

class requester
{
public:
    requester();
    ~requester();

    bool request(const parsing_config &config, data_for_monitoring& data_for_monitoring_);

private:
    Json::Value _root;
    char _error_buffer[CURL_ERROR_SIZE];
    std::string _buffer;

    CURL *_curl;
    CURLcode _result;

    //Fill in the lists of servers and ports from json file
    bool get_page(const parsing_config& config);
    bool response_analysis(data_for_monitoring& data_for_monitoring_);

    static int read_handle(char *data, size_t size, size_t nmemb, std::string* buffer);
};