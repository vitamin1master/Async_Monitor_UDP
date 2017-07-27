#include "stun_header.h"
#include <boost/asio.hpp>

stun_header::stun_header(uint16_t _mag_type, uint32_t _data_len, uint32_t _magic_cookie)
{
	req.msg_type = _mag_type;
	req.data_len = _data_len;
	req.magic_cookie = _magic_cookie;
	data = new char[_data_len];
}
stun_header::stun_header(char* str, int length)
{
	data = new char[length];
	req.msg_type = *reinterpret_cast<short *>(&str[0]);
	//0x2112A442 is is a fixed value for stun request (magic_cookie)
	req.magic_cookie = htonl(0x2112A442);
	for (int j = 0, i = 20; i < length; i++, j++)
	{
		data[j] = str[i];
	}
	req.data_len = length - 20;
}
char* stun_header::mapped_address() const
{
	if (req.msg_type != htons(0x0101))
		return nullptr;
	int maxAddress = 20;
	short attr_length = 0;
	for (int i = 0; i < req.data_len; i += attr_length + 4)
	{
		short attr_type = ntohs(*reinterpret_cast<short *>(&data[i]));
		attr_length = htons(*reinterpret_cast<short *>(&data[i + 2]));
		if (attr_type == 0x0020)
		{
			short port = ntohs(*reinterpret_cast<short *>(&data[i + 6]));
			port ^= 0x2112;
			char* str = new char[maxAddress];
			snprintf(str, maxAddress, "%d.%d.%d.%d:%d", std::abs(data[i + 8] ^ 0x21), std::abs(data[i + 9] ^ 0x12),
				std::abs(data[i + 10] ^ 0xA4), std::abs(data[i + 11] ^ 0x42), std::abs(port));
			return str;
		}
	}
	return nullptr;
}
stun_request stun_header::get_request() const
{
	return req;
}

uint16_t& stun_header::get_msg_type()
{
	return req.msg_type;
}

uint16_t& stun_header::get_data_len()
{
	return req.data_len;
}

uint32_t & stun_header::get_magic_cookie()
{
	return req.magic_cookie;
}

uint32_t & stun_header::get_id(int index)
{
	if (index <= 0 || index >= 2)
	{
		std::error_code e(static_cast<int>(std::errc::invalid_argument),
			std::generic_category());
		throw new std::system_error(e);
	}
	return req.id[index];
}

stun_header::~stun_header()
{
	delete[]data;
}