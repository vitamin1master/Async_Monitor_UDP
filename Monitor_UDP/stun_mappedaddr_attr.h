#pragma once
#include <cstdint>
#pragma pack(push, 1)
struct stun_mappedaddr_attr
{
	uint16_t type;
	uint16_t length;
	uint8_t reserved;
	uint8_t family;
	uint16_t port;
	uint8_t id[4];
};
#pragma pack(pop)