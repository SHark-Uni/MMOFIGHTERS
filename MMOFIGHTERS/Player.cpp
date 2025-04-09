#include "Player.h"
#include "PlayerDefine.h"
#include "ContentDefine.h"

#include <stdlib.h>

using namespace Core;
using namespace Common;

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
	return (rand() % (RANGE_MOVE_BOTTOM - RANGE_MOVE_TOP) + RANGE_MOVE_TOP);
}

int Player::generateSpawnX() const
{
	return (rand() % (RANGE_MOVE_RIGHT - RANGE_MOVE_LEFT) + RANGE_MOVE_LEFT);
}
void Player::Move(const short x, const short y)
{
	// �ϳ��� ���������� ������ �������� �����ؾ���. 
	// �̰� MAX���� üũ�� ������. �̷���� �밢������ ���� �ո�

	int nextX = _X + x;
	int nextY = _Y + y;

	//���࿡, �������� ������ ���� ���̶�� �������. (������ x)
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
	//����� ������ ������ �޴� ȸ������ ������ ����. ���� �ذ��� �˻� ��
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
		|| x > static_cast<short>(RANGE_MOVE_RIGHT)
		|| y < static_cast<short>(RANGE_MOVE_TOP)
		|| y > static_cast<short>(RANGE_MOVE_BOTTOM)
	{
		return true;
	}
	return false;
}
