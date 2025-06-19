#pragma once

#pragma comment(lib, "winmm")
#define _WINSOCKAPI_
#include <windows.h>
#include <vector>
#include <time.h>
#include "Sector.h"

namespace Core
{
	constexpr int FRAME = 25;
	constexpr int TIME_PER_FRAME = 1000 / FRAME;
	constexpr int FRAME_LOG_TIME = 10000;
	class GameServer;
	class FrameManager
	{
	public:
		inline void InitSector(Sector* sector)
		{
			tmp_sector = sector;
		}
		inline void InitGameserver(GameServer* gameserver)
		{
			debugGameserver = gameserver;
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

		void DisplayFrameInfo();

	private:
		DWORD _PrevTick;
		DWORD _FrameOutPutTime;

		DWORD _MaxDeltaTime;
		DWORD _MinDeltaTime;

		GameServer* debugGameserver;
		Sector* tmp_sector;
	};
}
