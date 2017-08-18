#include "headers/connection_test_packet.h"

connection_test_packet::connection_test_packet(const connection_info& info_,
											   boost::asio::io_service& service) : stop_indicator(false),
																				   read_indicator(false), info(info_),
																				   timer(service),
																				   count_send_request(0),
																				   end_point(boost::asio::ip::address::from_string(info.server_id), info.server_port)
{

}

connection_test_packet::~connection_test_packet()
{
}

bool connection_test_packet::is_connect_sougth(const std::shared_ptr<const connection_test_packet>& connection, const boost::asio::ip::udp::endpoint& end_point)
{
	return connection->end_point == end_point;
}