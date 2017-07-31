#pragma once
#include <stdint.h>
#include <boost/asio.hpp>
#include "stun_request.h"

class stun_header
{
	stun_request req;
public:
	stun_header(uint16_t _mag_type = 0, uint32_t _data_len = 0, uint32_t _magic_cookie = htonl(0x2112A442));
	stun_header(char* str, int length);
	~stun_header();

	char* mapped_address() const;
	//Get stun_request
	stun_request get_request() const;

	uint16_t& get_msg_type();
	uint16_t& get_data_len();
	uint32_t& get_magic_cookie();
	uint32_t& get_id(int index);
	std::vector<char> data;
};

