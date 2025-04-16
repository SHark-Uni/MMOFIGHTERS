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
			_deltaTime.reserve(64);
			_PrevTick = timeGetTime();
			_FrameOutPutTime = _PrevTick;
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
			DWORD ret = curTime - _PrevTick;

			if (ret < TIME_PER_FRAME)
			{
				_deltaTime.push_back(TIME_PER_FRAME);
			}
			else
			{
				_deltaTime.push_back(ret);
			}
			_PrevTick += TIME_PER_FRAME;
			return ret;
		}

		inline void DisplayFrameInfo()
		{
			DWORD curTime = ::timeGetTime();
			DWORD maxDeltaTime = 0;
			DWORD minDeltaTime = INT_MAX;
			if (_FrameOutPutTime - curTime >= 1000)
			{
				_FrameOutPutTime = curTime;
				size_t len = _deltaTime.size();
				for (int i = 0; i < len; i++)
				{
					if (maxDeltaTime < _deltaTime[i])
					{
						maxDeltaTime = _deltaTime[i];
					}

					if (minDeltaTime > _deltaTime[i])
					{
						minDeltaTime = _deltaTime[i];
					}
				}
				struct tm t;
				time_t timer;
				_time64(&timer);

				localtime_s(&t, &timer);

				printf("[%d : %d : %d] MIN_FRAME : %d | MAX_FRAME : %d \n", t.tm_hour, t.tm_min, t.tm_sec, 1000 / maxDeltaTime, 1000 / minDeltaTime);
			}
		}
	private:
		DWORD _PrevTick;
		DWORD _FrameOutPutTime;
		std::vector<DWORD> _deltaTime;
	};
}
