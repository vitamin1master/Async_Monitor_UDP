#pragma once
#include <stdint.h>
#include "stun_mappedaddr_attr.h"

#define msg_type_binding_request  htons(1)
#define msg_type_binding_response htons(0x0101)
#define fixed_magic_cookie htonl(0x2112A442)
#define attr_type_xor_mappaddr htons(0x0020)
#define attr_family_ipv4 0x01
#define attr_family_ipv6 0x02
#define msg_hdr_length 20

#pragma pack(push, 1)
struct stun_response
{
	uint16_t msg_type;
	uint16_t data_len;
	uint32_t magic_cookie;
	uint32_t transaction_id[3];
	stun_mappedaddr_attr mappedaddr_attr;
};
#pragma pack(pop)