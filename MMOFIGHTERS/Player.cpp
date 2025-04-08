#include "Player.h"
#include "PlayerDefine.h"

#include <stdlib.h>

using namespace Core;

void Player::Init(const int playerId, const int sessionId)
{
	_PlayerId = playerId;
	_SessionId = sessionId;

	_Action = static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION);
	_Direction = static_cast<int>(PLAYER_DEFAULT::DEFAULT_DIR);
	_IsAlive = true;

	_X = generateSpawnX();
	_Y = generateSpawnY();

	_Hp = static_cast<int>(PLAYER_DEFAULT::PLAYER_HP);
}

int Player::generateSpawnY() const
{
	return (rand() % (static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM) - static_cast<int>(MAX_MAP_BOUNDARY::TOP)) + static_cast<int>(MAX_MAP_BOUNDARY::TOP));
}

int Player::generateSpawnX() const
{
	return (rand() % (static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) - static_cast<int>(MAX_MAP_BOUNDARY::LEFT)) + static_cast<int>(MAX_MAP_BOUNDARY::LEFT));
}
void Player::Move(const short x, const short y)
{
	// 하나라도 막혀있으면 못가는 로직으로 변경해야함. 
	// 이건 MAX값만 체크한 로직임. 이럴경우 대각선으로 가면 뚫림

	int nextX = _X + x;
	int nextY = _Y + y;

	//만약에, 다음으로 움직일 곳이 벽이라면 멈춰야함. (움직임 x)
	if (CheckWallCollision(nextX, nextY))
	{
		return;
	}

	_X += x;
	_Y += y;
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
	if (x < static_cast<short>(MAX_MAP_BOUNDARY::LEFT)
		|| x > static_cast<short>(MAX_MAP_BOUNDARY::RIGHT)
		|| y < static_cast<short>(MAX_MAP_BOUNDARY::TOP)
		|| y > static_cast<short>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return true;
	}
	return false;
}
