#include "Player.h"
#include "ContentDefine.h"

#include <stdlib.h>

using namespace Core;
using namespace Common;
using namespace NetLib;

void Player::Init(const PLAYER_KEY playerId, const SESSION_KEY sessionId)
{
	_PlayerId = playerId;
	_SessionId = sessionId;

	_Action = static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION);
	_Direction = static_cast<int>(PLAYER_DEFAULT::DEFAULT_DIR);
	_IsMoveSector = false;
	_IsAlive = true;

	_X = generateSpawnX();
	_Y = generateSpawnY();

	_PrevSectorPos = { _X / SECTOR_WIDTH , _Y / SECTOR_HEIGHT };
	_CurSectorPos = { _X / SECTOR_WIDTH , _Y / SECTOR_HEIGHT };

	_Hp = static_cast<int>(PLAYER_DEFAULT::PLAYER_HP);
}

int Player::generateSpawnY() const
{
#ifdef GAME_DEUBG
	return 400;
#else
	return (rand() % (RANGE_MOVE_BOTTOM - RANGE_MOVE_TOP) + RANGE_MOVE_TOP);
#endif
}

int Player::generateSpawnX() const
{
#ifdef GAME_DEUBG
	return 150;
#else
	return (rand() % (RANGE_MOVE_RIGHT - RANGE_MOVE_LEFT) + RANGE_MOVE_LEFT);
#endif
}
void Player::Move(const short x, const short y)
{
	// 하나라도 막혀있으면 못가는 로직으로 변경해야함. 
	// 이건 MAX값만 체크한 로직임. 이럴경우 대각선으로 가면 뚫림

	//만약에, 다음으로 움직일 곳이 벽이라면 멈춰야함. (움직임 x)
	if (CheckWallCollision(_X + x, _Y + y))
	{
		return;
	}
	
	SECTOR_POS nextPos = { (_X + x) / SECTOR_WIDTH, (_Y + y) / SECTOR_WIDTH };
	//섹터 변화가 있다면 Update 
	if (CheckUpdateSector(_CurSectorPos.x, _CurSectorPos.y, nextPos.x, nextPos.y) == true)
	{
		_PrevSectorPos = _CurSectorPos;
		_CurSectorPos = nextPos;

		_X += x;
		_Y += y;
		_IsMoveSector = true;
	}
	else
	{
		_X += x;
		_Y += y;
	}
	return;
}

void Player::Attacked(const int damage)
{
	//양수로 음수로 데미지 받는 회복같은 개념은 없음. 양의 극값은 검사 ㄴ
	_Hp -= damage;
	if (_Hp <= 0)
	{
		_IsAlive = false;
		_Hp = 0;
	}
	return;
}

bool Core::Player::CheckWallCollision(const int x, const int y)
{
	if (x < static_cast<short>(RANGE_MOVE_LEFT)
		|| x >= static_cast<short>(RANGE_MOVE_RIGHT)
		|| y < static_cast<short>(RANGE_MOVE_TOP)
		|| y >= static_cast<short>(RANGE_MOVE_BOTTOM))
	{
		return true;
	}
	return false;
}
