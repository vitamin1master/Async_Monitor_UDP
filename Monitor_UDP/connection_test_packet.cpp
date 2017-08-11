#include "connection_test_packet.h"

connection_test_packet::connection_test_packet(connection_info info_, boost::asio::io_service& service_, boost::function<void(std::shared_ptr<connection_test_packet>&)> func) :
stop_indicator(false), read_indicator(false), info(info_), timer(service_), count_send_request(0),response_buf(nullptr), 
end_point(boost::asio::ip::address::from_string(info.server_id), info.server_port),connection_stop_handler(func)
{

}


connection_test_packet::~connection_test_packet()
{
	if(response_buf)
	{
		delete[]response_buf;
	}
}

bool connection_test_packet::isConnectSougth(const std::shared_ptr<connection_test_packet>& connection, const boost::asio::ip::udp::endpoint& end_point)
{
	return connection->end_point == end_point;
}
