#pragma once

#pragma comment(lib, "winmm")
#include <windows.h>
#include <vector>
#include <time.h>

namespace Core
{
	constexpr int FRAME = 50;
	constexpr int TIME_PER_FRAME = 1000 / FRAME;
	class GameServer;
	class FrameManager
	{
	public:
		inline void SetTimer()
		{
			_deltaTime.reserve(64);
			_NextTick = timeGetTime();
			_FrameOutPutTime = _NextTick;
		}
		inline int fixedUpdate(GameServer* gameServer)
		{

			_NextTick += TIME_PER_FRAME;
			int sleepTime =  _NextTick - ::timeGetTime();
			//_deltaTime.push_back();


			return sleepTime;
		}
		inline void DisplayFrameInfo()
		{
			if (_FrameOutPutTime - ::timeGetTime() >= 1000)
			{
				//·Î±ëÁ¤º¸ Âï±â
				size_t len = _deltaTime.size();
				int max = INT_MIN;

				for (int i = 0; i < len; i++)
				{
					if (max < _deltaTime[i])
					{
						max = _deltaTime[i];
					}
				}

				struct tm t;
				time_t timer;
				_time64(&timer);

				localtime_s(&t, &timer);

				printf("[%d:%d:%d] | MIN FRAME : %d \n",t.tm_hour,t.tm_min,t.tm_sec, 1000/max);
				_deltaTime.clear();
			}
		}
		
	private:
		GameServer* _gameServer;
		DWORD _NextTick;

		DWORD _FrameOutPutTime;
		DWORD _PureTime;
		std::vector<DWORD> _deltaTime;
	};
}
