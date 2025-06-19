#include "Gameserver.h"
#include "FrameManager.h"

using namespace Core;

void Core::FrameManager::DisplayFrameInfo()
{
	DWORD curTime = ::timeGetTime();
	if (curTime - _FrameOutPutTime >= FRAME_LOG_TIME)
	{
		_FrameOutPutTime = curTime;
		struct tm t;
		time_t timer;
		_time64(&timer);

		localtime_s(&t, &timer);

		if (_MinDeltaTime < TIME_PER_FRAME)
		{
			_MinDeltaTime = TIME_PER_FRAME;
		}

		if (_MaxDeltaTime < TIME_PER_FRAME)
		{
			_MaxDeltaTime = TIME_PER_FRAME;
		}
		int sum = 0;
		for (int i = 0; i < 50; i++)
		{
			for (int j = 0; j < 50; j++)
			{
				sum += tmp_sector->_Sector[i][j].size();
			}
		}
		size_t sessionCnt = debugGameserver->_Sessions.size();
		printf("[%d : %d : %d] MIN_FRAME : %d | MAX_FRAME : %d | Sector TOTAL : %d | sessionCnt : %lld\n",
			t.tm_hour, t.tm_min,
			t.tm_sec,
			1000 / _MaxDeltaTime,
			1000 / _MinDeltaTime,
			sum,
			sessionCnt
			);
		_MinDeltaTime = UINT_MAX;
		_MaxDeltaTime = 0;
	}
}
