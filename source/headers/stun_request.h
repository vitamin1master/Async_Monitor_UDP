#pragma once
#include <stdint.h>

struct stun_request
{
	uint16_t msg_type;
	uint16_t data_len;
	uint32_t magic_cookie;
	uint32_t id[3];
};
