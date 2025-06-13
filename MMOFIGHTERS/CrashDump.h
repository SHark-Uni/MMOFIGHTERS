#pragma once
#define _WINSOCKAPI_

#pragma comment(lib, "Dbghelp.lib")

#include <windows.h>
#include <iostream>
#include <DbgHelp.h>

namespace Common
{
	class Crashdump
	{
	public:
		Crashdump();

		static void crash();
		static LONG WINAPI crashfilter(__in PEXCEPTION_POINTERS exception_pointer);
		static void setMyErrorHandler();

		//custom Error Handler 
		static void custom_InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
		{
			crash();
			return;
		}
		static int custom_ReportHook(int ireposttype, char* message, int* returnvalue)
		{
			crash();
			return true;
		}
		static void custom_pureCallHandler(void)
		{
			crash();
			return;
		}
	private:

	};
}