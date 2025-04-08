#pragma once

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
	private:
		int _PlayerId;
		int _SessionId;

		int _Action;
		char _Direction;
		bool _IsAlive;

		short _X;
		short _Y;

		char _Hp;
	};
}