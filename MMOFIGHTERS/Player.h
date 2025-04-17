#pragma once

#include "Sector.h" 

namespace Core
{
	class Player
	{
	public:
		void Init(const int playerId, const int sessionId);
		int generatePlayerId()
		{
			static int playerId = 0;
			return playerId++;
		}

		int generateSpawnX() const;
		int generateSpawnY() const;

		inline bool IsAlive() const
		{
			return _IsAlive;
		}
		inline void SetPlayerDeath()
		{
			_IsAlive = false;
		}
		void Move(const short x, const short y);
		void Attacked(const int damage);

		bool CheckWallCollision(const int x, const int y);

		inline void SetX(short x)
		{
			_X = x;
		}
		inline void SetY(short y)
		{
			_Y = y;
		}
		inline void SetAction(int action)
		{
			_Action = action;
			return;
		}
		inline void SetDirection(char direction)
		{
			_Direction = direction;
		}
		inline char GetDirection() const
		{
			return _Direction;
		}
		inline int GetAction() const
		{
			return _Action;
		}
		inline int GetPlayerId() const
		{
			return _PlayerId;
		}
		inline int GetSessionId() const
		{
			return _SessionId;
		}
		inline int GetHp() const
		{
			return _Hp;
		}
		inline short GetX() const
		{
			return _X;
		}
		inline short GetY() const
		{
			return _Y;
		}

		inline bool CheckUpdateSector(int curX, int curY, int nextX, int nextY)
		{
			if (curX == nextX && curY == nextY)
			{
				return false;
			}
			return true;
		}

		inline void SetSector(SECTOR_POS sector)
		{
			_CurSectorPos = sector;
		}
		inline SECTOR_POS GetSector() const
		{
			return _CurSectorPos;
		}

		inline void SetPrevSector(SECTOR_POS sector)
		{
			_PrevSectorPos = sector;
		}
		inline SECTOR_POS GetPrevSector() const
		{
			return _PrevSectorPos;
		}

		inline void SetTimeOut(unsigned long time)
		{
			_LastTime = time;
		}

		inline unsigned long GetTimeOut()
		{
			return _LastTime;
		}
		inline bool IsMoveSector() const
		{
			return _IsMoveSector;
		}

		inline void MoveSectorIsDone()
		{
			_IsMoveSector = false;
		}
	private:
		int _PlayerId;
		int _SessionId;

		SECTOR_POS _CurSectorPos;
		SECTOR_POS _PrevSectorPos;

		int _Action;
		char _Direction;
		bool _IsAlive;
		bool _IsMoveSector;

		short _X;
		short _Y;

		char _Hp;
		unsigned long _LastTime;
	};
}