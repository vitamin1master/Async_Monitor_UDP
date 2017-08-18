#pragma once

#include <vector>
#include "connection_info.h"

class imonitor
{
public:
    virtual void verification_result_monitoring(const std::vector<connection_info>& completed_connections_info_list)=0;
};