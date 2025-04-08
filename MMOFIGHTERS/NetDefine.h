#pragma once

namespace NetLib
{
	constexpr size_t SBUFFER_POOL_SIZE = 6000;
	constexpr size_t SBUFFER_DEFAULT_SIZE = 1536;
	constexpr size_t SESSION_POOL_SIZE = 6000;
	constexpr size_t RINGBUFFER_POOL_SIZE = 6000;
	constexpr size_t RINGBUFFER_QUEUE_SIZE = 8192;

	constexpr char SIGNITURE = 0x89;

#pragma pack(push, 1)
	typedef struct NETWORK_HEADER
	{
		char _Code;
		char _PayloadLen;
	}nheader_t;
#pragma pack(pop)
}