#pragma once
#define _WINSOCKAPI_

#include <windows.h>

namespace Common
{
	class Logger
	{
	public:
		static void Logging(int errorCode, int line, const WCHAR* cause);
		static void LoggingWithIP(int errorCode, int line, const WCHAR* cuase, const WCHAR* ip);
	};
}