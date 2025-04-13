#pragma once

#include "ContentDefine.h"

namespace Core
{
	class Player;
	/* For Sector */


	/*
	1�� Sector�� ������
	64x64 (�ϴ� �̰ɷ� �غ���)

	128x128 (2��° ������)
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
		//��� Sector�� x,y��  Sector Width,Height�� ������ �ٷ� ����. �ű⿡ ������ָ� ��.  
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

		/*�ϴ�, for������ prev Sector�� new Sector �� deleteSector, add_sector ���� ����, �����ٸ� ���̽��� ������ ����ȭ ���Ѻ���.*/
		inline void getUpdateSurroundSector(SECTOR_POS prev_sector, SECTOR_POS new_sector, SECTOR_SURROUND& delete_secotr, SECTOR_SURROUND& add_sector)
		{
			SECTOR_SURROUND prevAround;
			SECTOR_SURROUND curAround;

			getSurroundSector(prev_sector.x, prev_sector.y, prevAround);
			getSurroundSector(new_sector.x, new_sector.y, curAround);

			//prev�������� ��ġ�� �ʴ� �κ� -> delete sector 
			bool flag = false;
			int cnt = 0;
			for (int i = 0; i < prevAround._Count; i++)
			{
				for (int j = 0; j < curAround._Count; j++)
				{
					//���ٸ�, flag �����
					if (prevAround._Surround[i].x == curAround._Surround[j].x &&
						prevAround._Surround[i].y == curAround._Surround[j].y)
					{
						flag ^= 1;
						continue;
					}
				}

				if (!flag)
				{
					delete_secotr._Surround[cnt].x = prevAround._Surround[i].x;
					delete_secotr._Surround[cnt].y = prevAround._Surround[i].y;
					++cnt;
					flag ^= 1;
				}
			}
			delete_secotr._Count = cnt;

			//cur�������� ��ġ�� �ʴ� �κ� ->addsector
			flag = false;
			cnt = 0;
			for (int i = 0; i < curAround._Count; i++)
			{
				for (int j = 0; j < prevAround._Count; j++)
				{
					//���ٸ�, flag �����
					if (curAround._Surround[i].x == prevAround._Surround[j].x &&
						curAround._Surround[i].y == prevAround._Surround[j].y)
					{
						flag ^= 1;
						continue;
					}
				}

				if (!flag)
				{
					add_sector._Surround[cnt].x = curAround._Surround[i].x;
					add_sector._Surround[cnt].y = curAround._Surround[i].y;
					++cnt;
					flag ^= 1;
				}
			}
			add_sector._Count = cnt;
		}
	private:
		std::list<Player*> _Sector[SECTOR_MAX_ROW][SECTOR_MAX_COLUMN];
	};
}



