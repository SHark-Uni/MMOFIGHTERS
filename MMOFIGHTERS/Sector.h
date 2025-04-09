#pragma once

#include "ContentDefine.h"

namespace Core
{
	class Player;
	/* For Sector */


	/*
	1개 Sector의 사이즈
	64x64 (일단 이걸로 해보자)

	128x128 (2번째 선택지)
	*/

	constexpr int SECTOR_WIDTH = 64;
	constexpr int SECTOR_HEIGHT = 64;
	constexpr int SECTOR_MAX_COLUMN = Common::MAX_MAP_RANGE / SECTOR_WIDTH;
	constexpr int SECTOR_MAX_ROW = Common::MAX_MAP_RANGE / SECTOR_HEIGHT;
	typedef struct SECTOR_POS
	{
		int x; // column 
		int y; // row 
	}sector_pos_t;
	typedef struct SECTOR_SURROUND
	{
		int _Count;
		struct SECTOR_POS _Surround[9];
	};

	class Sector
	{
	private:
		//사실 Sector의 x,y는  Sector Width,Height를 나누면 바로 나옴. 거기에 등록해주면 됨.  
		inline void getSurroundSector(int sector_x, int sector_y, SECTOR_SURROUND& sectorSurround)
		{
			int dx[3] = { -1,0,1 };
			int dy[3] = { -1,0,1 };

			int nx;
			int ny;
			int cnt = 0;
			for (int i = 0; i < 3; i++)
			{
				nx = sector_x + dx[i];
				if (nx < 0 || nx > SECTOR_MAX_COLUMN)
				{
					continue;
				}
				for(int j = 0; j < 3; j++)
				{
					ny = sector_y + dy[i];
					if (ny < 0 || ny > SECTOR_MAX_ROW)
					{
						continue;
					}
					sectorSurround._Surround[cnt].x = nx;
					sectorSurround._Surround[cnt].y = ny;
					++cnt;
				}
			}
			sectorSurround._Count = cnt;
			return;
		}

		inline void enrollPlayer(int sector_x, int sector_y, Player* pTarget)
		{
			_Sector[sector_y][sector_x].push_back(pTarget);
		}

		inline void dropOutPlayer(int sector_x, int sector_y, Player* pTarget)
		{
			_Sector[sector_y][sector_x].remove(pTarget);
		}

		/*일단, for문으로 prev Sector와 new Sector 중 deleteSector, add_sector 구한 다음, 느리다면 케이스를 나눠서 최적화 시켜보자.*/
		inline void getUpdateSurroundSector(SECTOR_POS prev_sector, SECTOR_POS new_sector, SECTOR_SURROUND& delete_secotr, SECTOR_SURROUND& add_sector)
		{
			SECTOR_SURROUND prevAround;
			SECTOR_SURROUND curAround;

			getSurroundSector(prev_sector.x, prev_sector.y, prevAround);
			getSurroundSector(new_sector.x, new_sector.y, curAround);

			//교집합 부분 뽑아내고, 

		}
	private:
		std::list<Player*> _Sector[SECTOR_MAX_ROW][SECTOR_MAX_COLUMN];
	};

	

}



