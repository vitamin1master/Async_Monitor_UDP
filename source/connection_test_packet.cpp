#include "headers/connection_test_packet.h"

connection_test_packet::connection_test_packet(const connection_info& info_, boost::asio::io_service& service, const boost::function<void(const std::shared_ptr<const connection_test_packet>&)>& func) :
stop_indicator(false), read_indicator(false), info(info_), timer(service), count_send_request(0),response_buf(nullptr),
end_point(boost::asio::ip::address::from_string(info.server_id), info.server_port),connection_stop_handler(func)
{

}

connection_test_packet::~connection_test_packet()
{
	if(response_buf != nullptr)
	{
		delete[]response_buf;
	}
}

bool connection_test_packet::is_connect_sougth(const std::shared_ptr<const connection_test_packet>& connection, const boost::asio::ip::udp::endpoint& end_point)
{
	return connection->end_point == end_point;
}