#pragma once
#define _WINSOCKAPI_
#include <windows.h>

namespace Common
{
	typedef struct CONFIG
	{
		unsigned short _Port;
	}CONFIG_t;

	constexpr int FRAME = 50;
	constexpr int TIME_PER_FRAME = 1000 / FRAME;
}