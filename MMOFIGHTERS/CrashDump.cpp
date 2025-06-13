#include "CrashDump.h"

using namespace Common;

static SRWLOCK _Dumpmutex;

Common::Crashdump::Crashdump()
{

	_invalid_parameter_handler oldHandler;
	_invalid_parameter_handler newHandler;

	//Change ErrorHandler CRT -> custom
	newHandler = custom_InvalidParameterHandler;
	oldHandler = _set_invalid_parameter_handler(newHandler);
	_CrtSetReportHook(custom_ReportHook);
	_set_purecall_handler(custom_pureCallHandler);
	//CRT Warning Off
	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);
	::InitializeSRWLock(&_Dumpmutex);
	setMyErrorHandler();
}

void Common::Crashdump::crash()
{
	__debugbreak();
}

LONG __stdcall Common::Crashdump::crashfilter(__in PEXCEPTION_POINTERS exception_pointer)
{

	SYSTEMTIME curTime;

	WCHAR fileName[256] = { 0, };

	AcquireSRWLockExclusive(&_Dumpmutex);
	GetLocalTime(&curTime);
	wsprintf(fileName, L"dump\\DUMP_%d%02d%02d_%02d-%02d-%02d.dmp",
		curTime.wYear,
		curTime.wMonth,
		curTime.wDay,
		curTime.wHour,
		curTime.wMinute,
		curTime.wSecond
	);
	wprintf(fileName);

	wprintf(L"===========================================================\n");
	wprintf(L"!!CRASH!! !!CRASH!! !!CRASH!! !!CRASH!! !!CRASH!! !!CRASH!!\n");
	wprintf(L"===========================================================\n");
	wprintf(L"Saving DumpFile...\n");


	//printf("SRWLOCK Enter\n");
	HANDLE hDumpFile = ::CreateFile(fileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;

		exceptionInfo.ThreadId = ::GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = exception_pointer;
		exceptionInfo.ClientPointers = true;

		//MiniDump는 메모리를 Dump작성
		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&exceptionInfo,
			NULL,
			NULL
		);

		CloseHandle(hDumpFile);
		wprintf(L"Save DumpFile Finish\n");
	}
	ReleaseSRWLockExclusive(&_Dumpmutex);
	return EXCEPTION_EXECUTE_HANDLER;
}

void Common::Crashdump::setMyErrorHandler()
{
	SetUnhandledExceptionFilter(crashfilter);
}
