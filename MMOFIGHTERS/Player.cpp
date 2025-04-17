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

	_PrevSectorPos = { _X / SECTOR_WIDTH , _Y / SECTOR_HEIGHT };
	_CurSectorPos = { _X / SECTOR_WIDTH , _Y / SECTOR_HEIGHT };

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

	//���࿡, �������� ������ ���� ���̶�� �������. (������ x)
	if (CheckWallCollision(_X + x, _Y + y))
	{
		return;
	}
	
	//���� ��ȭ�� �ִٸ� Update 
	if (CheckUpdateSector(_CurSectorPos.x, _CurSectorPos.y, (_X + x) / SECTOR_WIDTH, (_Y + y) / SECTOR_HEIGHT))
	{
		_PrevSectorPos = _CurSectorPos;
		_CurSectorPos.x = (_X + x) / SECTOR_WIDTH;
		_CurSectorPos.y = (_Y + y) / SECTOR_HEIGHT;
		_X += x;
		_Y += y;
		_IsMoveSector = true;
	}
	else
	{
		_X += x;
		_Y += y;
		_IsMoveSector = false;
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
		|| x > static_cast<short>(RANGE_MOVE_RIGHT)
		|| y < static_cast<short>(RANGE_MOVE_TOP)
		|| y > static_cast<short>(RANGE_MOVE_BOTTOM))
	{
		return true;
	}
	return false;
}
