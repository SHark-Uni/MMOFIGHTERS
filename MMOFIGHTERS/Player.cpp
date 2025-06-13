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
	// �ϳ��� ���������� ������ �������� �����ؾ���. 
	// �̰� MAX���� üũ�� ������. �̷���� �밢������ ���� �ո�

	//���࿡, �������� ������ ���� ���̶�� �������. (������ x)
	if (CheckWallCollision(_X + x, _Y + y))
	{
		return;
	}
	
	SECTOR_POS nextPos = { (_X + x) / SECTOR_WIDTH, (_Y + y) / SECTOR_WIDTH };
	//���� ��ȭ�� �ִٸ� Update 
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
		|| x >= static_cast<short>(RANGE_MOVE_RIGHT)
		|| y < static_cast<short>(RANGE_MOVE_TOP)
		|| y >= static_cast<short>(RANGE_MOVE_BOTTOM))
	{
		return true;
	}
	return false;
}
