#pragma once

#pragma comment(lib, "winmm")
#include <windows.h>
#include <vector>
#include <time.h>

namespace Core
{
	constexpr int FRAME = 25;
	constexpr int TIME_PER_FRAME = 1000 / FRAME;
	constexpr int FRAME_LOG_TIME = 10000;
	class FrameManager
	{
	public:
		inline void InitSector(Sector* sector)
		{
			tmp_sector = sector;
		}
		inline void InitTimer()
		{
			_PrevTick = timeGetTime();
			_FrameOutPutTime = _PrevTick;
			_MinDeltaTime = UINT_MAX;
			_MaxDeltaTime = 0;
		}
		inline DWORD GetPrevTime() const
		{
			return _PrevTick;
		}
		inline void SetTimer(DWORD prevtick)
		{
			_PrevTick = prevtick;
		}
		inline void AddElaspedTime_pastFrame(int pastFrameCnt)
		{
			_PrevTick += TIME_PER_FRAME * pastFrameCnt;
		}
		inline DWORD CalculateTimeInterval()
		{
			DWORD curTime = ::timeGetTime();
			DWORD Interval = curTime - _PrevTick;

			if (_MinDeltaTime > Interval)
			{
				_MinDeltaTime = Interval;
			}
			if (_MaxDeltaTime < Interval)
			{
				_MaxDeltaTime = Interval;
			}
			_PrevTick += TIME_PER_FRAME;
			return Interval;
		}

		inline void DisplayFrameInfo()
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
				printf("[%d : %d : %d] MIN_FRAME : %d | MAX_FRAME : %d | Sector TOTAL : %d \n", t.tm_hour, t.tm_min, t.tm_sec, 1000 / _MaxDeltaTime, 1000 / _MinDeltaTime, sum);
				_MinDeltaTime = UINT_MAX;
				_MaxDeltaTime = 0;
			}
		}
	private:
		DWORD _PrevTick;
		DWORD _FrameOutPutTime;

		DWORD _MaxDeltaTime;
		DWORD _MinDeltaTime;
		Sector* tmp_sector;
	};
}
