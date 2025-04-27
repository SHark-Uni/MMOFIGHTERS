#pragma once

#include "ContentDefine.h"
#include <list>

namespace Core
{
	typedef struct SECTOR_POS
	{
		int x; // column 
		int y; // row 

		bool operator==(SECTOR_POS other)
		{
			if (x == other.x && y == other.y)
			{
				return true;
			}
			return false;
		}

	}sector_pos_t;
	typedef struct SECTOR_SURROUND
	{
		int _Count;
		struct SECTOR_POS _Surround[9];
	}sector_surround_t;
	
	class Player;
	class GameServer;
	class FrameManager;
	class Sector
	{
	public:
		//사실 Sector의 x,y는  Sector Width,Height를 나누면 바로 나옴. 거기에 등록해주면 됨.  
		inline void getSurroundSector(int sector_x, int sector_y, SECTOR_SURROUND& sectorSurround)
		{
			int dx[Common::SECTOR_COLUMN_SIZE] = { -1,0,1 };
			int dy[Common::SECTOR_ROW_SIZE] = { -1,0,1 };

			int nx;
			int ny;
			int cnt = 0;
			for (int i = 0; i < Common::SECTOR_COLUMN_SIZE; i++)
			{
				nx = sector_x + dx[i];
				if (nx < 0 || nx >= Common::SECTOR_MAX_COLUMN)
				{
					continue;
				}
				for (int j = 0; j < Common::SECTOR_ROW_SIZE; j++)
				{
					ny = sector_y + dy[j];
					if (ny < 0 || ny >= Common::SECTOR_MAX_ROW)
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
		inline void enrollPlayer(SECTOR_POS pos, Player* pTarget)
		{
			_Sector[pos.y][pos.x].push_back(pTarget);
		}
		
		/*내 현재 섹터기준 오른쪽 Sector를 탐색*/
		inline void getRightSideSector(const int sector_x, const int sector_y, SECTOR_SURROUND& rightSide)
		{
			int cnt = 0;
			for (int offsetX = 0; offsetX < 2; offsetX++)
			{
				if ((sector_x + offsetX) >= Common::SECTOR_MAX_COLUMN || (sector_x + offsetX) < 0)
				{
					continue;
				}
				for (int offsetY = -1; offsetY <= 1; offsetY++)
				{
					if ((sector_y + offsetY) < 0 || (sector_y + offsetY) >= Common::SECTOR_MAX_ROW)
					{
						continue;
					}
					rightSide._Surround[cnt].x = sector_x + offsetX;
					rightSide._Surround[cnt].y = sector_y + offsetY;
					++cnt;
				}
			}
			rightSide._Count = cnt;
		}
		inline void getLeftSideSector(const int sector_x, const int sector_y, SECTOR_SURROUND& leftSide)
		{
			int cnt = 0;
			for (int offsetX = 0; offsetX >= -1; offsetX--)
			{
				if ((sector_x + offsetX) >= Common::SECTOR_MAX_COLUMN || (sector_x + offsetX) < 0)
				{
					continue;
				}
				for (int offsetY = -1; offsetY <= 1; offsetY++)
				{
					if ((sector_y + offsetY) < 0 || (sector_y + offsetY) >= Common::SECTOR_MAX_ROW)
					{
						continue;
					}
					leftSide._Surround[cnt].x = sector_x + offsetX;
					leftSide._Surround[cnt].y = sector_y + offsetY;
					++cnt;
				}
			}
			leftSide._Count = cnt;
		}
		
		inline void dropOutPlayer(int sector_x, int sector_y, Player* pTarget)
		{
			_Sector[sector_y][sector_x].remove(pTarget);
		}
		inline void dropOutPlayer(SECTOR_POS pos, Player* pTarget)
		{
			_Sector[pos.y][pos.x].remove(pTarget);
		}

		/*일단, for문으로 prev Sector와 new Sector 중 deleteSector, add_sector 구한 다음, 느리다면 케이스를 나눠서 최적화 시켜보자.*/
		inline void getUpdateSurroundSector(SECTOR_POS prev_sector, SECTOR_POS new_sector, SECTOR_SURROUND& delete_secotr, SECTOR_SURROUND& add_sector)
		{
			SECTOR_SURROUND prevAround;
			SECTOR_SURROUND curAround;

			getSurroundSector(prev_sector.x, prev_sector.y, prevAround);
			getSurroundSector(new_sector.x, new_sector.y, curAround);

			//prev기준으로 겹치지 않는 부분 -> delete sector 
			bool IsDuplicated;
			int cnt = 0;
			for (int i = 0; i < prevAround._Count; i++)
			{
				IsDuplicated = false;
				for (int j = 0; j < curAround._Count; j++)
				{
					//중복되는 부분은 등록 x
					if (prevAround._Surround[i].x == curAround._Surround[j].x &&
						prevAround._Surround[i].y == curAround._Surround[j].y)
					{
						IsDuplicated = true;
						break;
					}
				}

				if (IsDuplicated == true)
				{
					continue;
				}
				//flag가 여전히 false인 경우
				delete_secotr._Surround[cnt].x = prevAround._Surround[i].x;
				delete_secotr._Surround[cnt].y = prevAround._Surround[i].y;
				++cnt;
			}
			delete_secotr._Count = cnt;

			//cur기준으로 겹치지 않는 부분 ->addsector
			cnt = 0;
			for (int i = 0; i < curAround._Count; i++)
			{
				IsDuplicated = false;
				for (int j = 0; j < prevAround._Count; j++)
				{
					//같다면, flag 세우고
					if (curAround._Surround[i].x == prevAround._Surround[j].x &&
						curAround._Surround[i].y == prevAround._Surround[j].y)
					{
						IsDuplicated = true;
						break;
					}
				}
				if (IsDuplicated)
				{
					continue;
				}
				add_sector._Surround[cnt].x = curAround._Surround[i].x;
				add_sector._Surround[cnt].y = curAround._Surround[i].y;
				++cnt;
			}
			add_sector._Count = cnt;
		}
private:
		friend class GameServer;
		friend class FrameManager;
		std::list<Player*> _Sector[Common::SECTOR_MAX_ROW][Common::SECTOR_MAX_COLUMN];
	};
}



