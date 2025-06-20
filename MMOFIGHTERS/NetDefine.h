#pragma once

namespace NetLib
{
	typedef unsigned long long SESSION_KEY;

	constexpr size_t SBUFFER_POOL_SIZE = 80000;
	constexpr size_t SBUFFER_DEFAULT_SIZE = 1536;

	constexpr size_t SESSION_POOL_SIZE = 12500;
	constexpr int SESSION_RESERVE_SIZE = 12500;
	
	constexpr size_t RINGBUFFER_POOL_SIZE = 20040;
	constexpr size_t RINGBUFFER_QUEUE_SIZE = 25000;
}