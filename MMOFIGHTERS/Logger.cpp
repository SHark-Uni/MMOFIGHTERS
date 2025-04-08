#include <iostream>
#include <time.h>
#include <cassert>
#include "Logger.h"

using namespace Common;

void Logger::Logging(int errorCode, int line, const WCHAR* cause)
{
	const int fileNameLength = 64;
	const int errorMsgLen = 256;
	struct tm t;
	time_t timer;
	_time64(&timer);

	localtime_s(&t, &timer);
	FILE* fp;

	WCHAR FILENAME[fileNameLength] = { 0 , };
	WCHAR* ErrorMessage = new WCHAR[errorMsgLen];

	swprintf_s(FILENAME, fileNameLength, L"Logger_%dy_%dm_%d day_%dh%dm.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_wday,
		t.tm_hour,
		t.tm_min);

	_wfopen_s(&fp, FILENAME, L"wb");
	if (fp == NULL)
	{
		return;
	}
	fputwc(0xFEFF, fp);

	swprintf_s(ErrorMessage, errorMsgLen, L"Why : %s | ErrorCode : %d | Line : %d \n",
		cause,
		errorCode,
		line);
	fwprintf(fp, ErrorMessage);

	delete[] ErrorMessage;
	fclose(fp);
}
void Logger::LoggingWithIP(int errorCode, int line, const WCHAR* cause, const WCHAR* ip)
{
	const int fileNameLength = 64;
	const int errorMsgLen = 256;
	//어느 세션인지도 기록을 해놓을까?
	struct tm t;
	time_t timer;
	_time64(&timer);

	localtime_s(&t, &timer);
	FILE* fp;

	WCHAR FILENAME[fileNameLength] = { 0 , };
	WCHAR* ErrorMessage = new WCHAR[errorMsgLen];

	swprintf_s(FILENAME, fileNameLength, L"Logger_%dy_%dm_%d day_%dh%dm.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_wday,
		t.tm_hour,
		t.tm_min);

	_wfopen_s(&fp, FILENAME, L"wb");
	if (fp == NULL)
	{
		return;
	}
	fputwc(0xFEFF, fp);

	swprintf_s(ErrorMessage, errorMsgLen, L"ip : %s | Why : %s | ErrorCode : %d | Line : %d \n",
		ip,
		cause,
		errorCode,
		line);

	fwprintf(fp, ErrorMessage);

	delete[] ErrorMessage;
	fclose(fp);
}