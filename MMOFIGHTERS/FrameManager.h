#pragma once

#pragma comment(lib, "winmm")
#include <windows.h>
#include <vector>
#include <time.h>

namespace Core
{
	constexpr int FRAME = 50;
	constexpr int TIME_PER_FRAME = 1000 / FRAME;
	class FrameManager
	{
	public:
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
			if (curTime - _FrameOutPutTime >= 1000)
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
				printf("[%d : %d : %d] MIN_FRAME : %d | MAX_FRAME : %d \n", t.tm_hour, t.tm_min, t.tm_sec, 1000 / _MaxDeltaTime, 1000 / _MinDeltaTime);
				_MinDeltaTime = UINT_MAX;
				_MaxDeltaTime = 0;
			}
		}
	private:
		DWORD _PrevTick;
		DWORD _FrameOutPutTime;

		DWORD _MaxDeltaTime;
		DWORD _MinDeltaTime;
	};
}
