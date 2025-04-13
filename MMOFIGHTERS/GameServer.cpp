#include "GameServer.h"
#include "MessageFormat.h"
#include "MessageBuilder.h"

#include "PlayerDefine.h"
#include "Logger.h"
#include "NetDefine.h"
#include "ObjectPool.h"
#include "ContentDefine.h"

#include <cassert>

using namespace NetLib;
using namespace Core;
using namespace Common;

GameServer::GameServer()
{
	_Players.reserve(PLAYER_RESERVER_SIZE);
	_keys.reserve(PLAYER_RESERVER_SIZE);
}

GameServer::~GameServer()
{

}
void Core::GameServer::registPlayerPool(ObjectPool<Player, PLAYER_POOL_SIZE, false>* pool)
{
	_PlayerPool = pool;
}
/*==================================
onAcceptProc���� ����
1. �÷��̾� ���� (Contents�ڵ忡��
2. �ٸ� �÷��̾�� �� �÷��̾� ���� �޽��� ������
3. ���� �÷��̾�� ������ ���� �޽��� ������
===================================*/
void GameServer::OnAcceptProc(const SESSION_KEY key)
{
	int playerKey;

	Player* newPlayer = _PlayerPool->allocate();
	playerKey = newPlayer->generatePlayerId();

	newPlayer->Init(playerKey, key);
	_keys.insert({ key, playerKey });
	_Players.insert({ playerKey, newPlayer });

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	//1. �� ĳ���� ���� �޽��� ���� (����)
	buildMsg_createMyCharacter(
		static_cast<int>(MESSAGE_DEFINE::RES_CREATE_MY_CHARACTER),
		playerKey,
		newPlayer->GetDirection(),
		newPlayer->GetX(),
		newPlayer->GetY(),
		newPlayer->GetHp(),
		sBuffer
	);
	SendUniCast(key, sBuffer, sBuffer->getUsedSize());

	sBuffer->clear();
	//2.�� ĳ���� ���� �޽��� ���Ϳ��� �����ֱ�
	buildMsg_createOtherCharacter(
		static_cast<int>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER), 
		playerKey, 
		newPlayer->GetDirection(), 
		newPlayer->GetX(), 
		newPlayer->GetY(), 
		newPlayer->GetHp(), 
		sBuffer
	);
	SendToSector(sBuffer, newPlayer);
	//SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//3. ���͸� ���鼭, ���Ϳ� �����ϴ� ĳ���͵� �������ִ� �޽��� ������.
	SECTOR_POS curSector = newPlayer->GetSector();
	SECTOR_SURROUND around;

	int nx;
	int ny;
	_pSector->getSurroundSector(curSector.x, curSector.y, around);
	for (int i = 0; i < around._Count; i++)
	{
		nx = around._Surround[i].x;
		ny = around._Surround[i].y;

		for (auto& OtherPlayer : _pSector->_Sector[ny][nx])
		{
			if (OtherPlayer->GetPlayerId() == newPlayer->GetPlayerId())
			{
				continue;
			}
			sBuffer->clear();
			buildMsg_createOtherCharacter(
				static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
				OtherPlayer->GetPlayerId(), 
				OtherPlayer->GetDirection(), 
				OtherPlayer->GetX(),
				OtherPlayer->GetY(),
				OtherPlayer->GetHp(),
				sBuffer
			);
			SendUniCast(key, sBuffer, sBuffer->getUsedSize());

			//�����̰� �ִٸ�
			if (OtherPlayer->GetAction() > 0)
			{
				sBuffer->clear();
				buildMsg_move_start(
					static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START), 
					OtherPlayer->GetPlayerId(), 
					OtherPlayer->GetAction(), 
					OtherPlayer->GetX(),
					OtherPlayer->GetY(),
					sBuffer
				);
				SendUniCast(key, sBuffer, sBuffer->getUsedSize());
			}
		}
	}
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::OnRecvProc(SerializeBuffer* message, const char msgType, SESSION_KEY key)
{
	//Header �����ϰ� payload �Ѱ�����
	switch (msgType)
	{
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_START):
		ReqMoveStartProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_STOP):
		ReqMoveStopProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_LEFT_HAND):
		ReqAttackLeftHandProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_RIGHT_HAND):
		ReqAttackRightHandProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_KICK):
		ReqAttackKickProc(message, key);
		break;
	default:
#ifdef GAME_DEBUG
		//TODO : ���� �������.
		printf("BAD REQUEST!\n");
#endif
		OnDestroyProc(key);
		break;
	}
	return;
}
/* ������ ���� �ϱ�! ���� ���̽��� �����غ���.*/
void GameServer::ReqMoveStartProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char action;
	unsigned short recvX;
	unsigned short recvY;

	*message >> action >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//�б� ����. ����ȭ ���� ���� �߸��Ѱ���. Ȥ��, �޽��� ũ�Ⱑ ���ǵ��� �������·� ����.
		DebugBreak();
	}

	//���� �Ѵ� �޽��� ����
	if (recvX > RANGE_MOVE_RIGHT || recvY > RANGE_MOVE_BOTTOM)
	{
		return;
	}
	//�̻��� ���� ����
	if (action < static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION) || action > static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM))
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;
	player->SetAction(action);
	//���⼳��
	switch (action)
	{
	case static_cast<int>(MOVE_DIRECTION::LEFT):
	case static_cast<int>(MOVE_DIRECTION::LEFT_TOP):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::LEFT));
		break;
	case static_cast<int>(MOVE_DIRECTION::RIGHT_TOP):
	case static_cast<int>(MOVE_DIRECTION::RIGHT):
	case static_cast<int>(MOVE_DIRECTION::RIGHT_BOTTOM):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::RIGHT));
		break;
	case static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::LEFT));
		break;
	default:
		break;
	}
	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();
	//�� ���� �� ������

	buildMsg_move_start(static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START), playerKey, action, recvX, recvY, sBuffer);
#ifdef GAME_DEBUG
	printf("============================================================\n");
	printf("MOVE START MESSAGE\n");
	printf("PLAYER ID : %d | SESSION ID : %d | PARAMETER KEY : %d |CUR_X : %hd  | CUR_Y : %hd |\n", player->GetPlayerId(), player->GetSessionId(), key, player->GetX(), player->GetY());
	printf("============================================================\n");
#endif
	SendToSector(sBuffer,player);
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::ReqMoveStopProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char direction;
	unsigned short recvX;
	unsigned short recvY;

	*message >> direction >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//�б� ����. ����ȭ ���� ���� �߸��Ѱ���. Ȥ��, �޽��� ũ�Ⱑ ���ǵ��� �������·� ����.
		DebugBreak();
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;

	short playerX = player->GetX();
	short playerY = player->GetY();

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	if (abs(recvX - playerX) > COORD_ERROR_TOLERANCE ||
		abs(recvY - playerY) > COORD_ERROR_TOLERANCE)
	{
#ifdef GAME_DEBUG
		printf("CURRENT X : %hd | CURRENT Y : %hd \n", playerX, playerY);
		printf("OUT OF BOUNDARY DISCONNECT!\n");
		WCHAR buffer[80] = { 0, };
		swprintf_s(buffer, L"PLAYER X : %hd | PLAYER Y : %hd | RECV X : %hd | RECV Y : %hd\n", playerX, playerY, recvX, recvY);
		Logger::Logging(-2, __LINE__, buffer);
#endif

		//OnDestroyProc(key);
	}

	if (recvX > RANGE_MOVE_RIGHT || recvY > RANGE_MOVE_BOTTOM)
	{
		return;
	}
	if (CheckDirection(direction) == false)
	{
		return;
	}

	player->SetX(recvX);
	player->SetY(recvY);
	player->SetDirection(direction);
	player->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));

#ifdef GAME_DEBUG
	printf("MOVE STOP MESSAGE\n");
	printf("PLAYER ID : %d | SESSION ID : %d | PARAM KEY : %d |CUR_X : %hd  | CUR_Y : %hd |\n", player->GetPlayerId(), player->GetSessionId(), key, player->GetX(), player->GetY());
#endif
	
	sBuffer->clear();

	//���꽺ž �޽��� ���� �� ������
	buildMsg_move_stop(static_cast<char>(MESSAGE_DEFINE::RES_MOVE_STOP), playerKey, direction, player->GetX(), player->GetY(), sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	_SbufferPool->deAllocate(sBuffer);
	return;
}

void GameServer::ReqAttackLeftHandProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char attackDir;
	unsigned short recvX;
	unsigned short recvY;

	*message >> attackDir >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//�б� ����. ����ȭ ���� ���� �߸��Ѱ���. Ȥ��, �޽��� ũ�Ⱑ ���ǵ��� �������·� ����.
		DebugBreak();
	}
	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;
	short myX = attacker->GetX();
	short myY = attacker->GetY();

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	//���� Message Send
	buildMsg_attack_lefthand(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), playerKey, attackDir, myX, myY, sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//���ݹ��� ����
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//���� �� �ڽ��� ������ �ȵ�.
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}
		int targetX = target->GetX();
		int targetY = target->GetY();
		if (CheckAttackInRange(
			myX,
			myY,
			ATTACK_LEFT_HAND_RANGE_X,
			ATTACK_LEFT_HAND_RANGE_Y,
			targetX, targetY, attackDir))
		{
			sBuffer->clear();
			target->Attacked(DAMAGE_LEFT_HAND);
			buildMsg_damage(static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), playerKey, target->GetPlayerId(), target->GetHp(), sBuffer);
			SendBroadCast(sBuffer, sBuffer->getUsedSize());
		}
	}
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::ReqAttackRightHandProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char attackDir;
	unsigned short recvX;
	unsigned short recvY;

	*message >> attackDir >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		DebugBreak();
	}
	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	short myX = attacker->GetX();
	short myY = attacker->GetY();


	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	//���� Message Send
	buildMsg_attack_righthand(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_RIGHT_HAND), playerKey, attackDir, myX, myY, sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//���ݹ��� ����
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//���� �� �ڽ��� ������ �ȵ�.
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}
		int targetX = target->GetX();
		int targetY = target->GetY();
		if (CheckAttackInRange(
			myX,
			myY,
			ATTACK_RIGHT_HAND_RANGE_X,
			ATTACK_RIGHT_HAND_RANGE_Y,
			targetX, targetY, attackDir))
		{
			sBuffer->clear();

			target->Attacked(DAMAGE_RIGHT_HAND);
			buildMsg_damage(static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), playerKey, target->GetPlayerId(), target->GetHp(), sBuffer);

			SendBroadCast(sBuffer, sBuffer->getUsedSize());
		}
	}
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::ReqAttackKickProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char attackDir;
	unsigned short recvX;
	unsigned short recvY;

	*message >> attackDir >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		DebugBreak();
	}

	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	//���� Message Send
	buildMsg_attack_kick(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_KICK), playerKey, attackDir, attacker->GetX(), attacker->GetY(), sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//���ݹ��� ����
	for (auto& player : _Players)
	{
		Player* target = player.second;
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}
		int targetX = target->GetX();
		int targetY = target->GetY();
		if (CheckAttackInRange(
			attacker->GetX(),
			attacker->GetY(),
			ATTACK_KICK_X,
			ATTACK_KICK_Y,
			targetX, targetY, attackDir))
		{
			sBuffer->clear();
			target->Attacked(DAMAGE_KICK);
			buildMsg_damage(static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), playerKey, target->GetPlayerId(), target->GetHp(), sBuffer);
			SendBroadCast(sBuffer, sBuffer->getUsedSize());

		}
	}
	_SbufferPool->deAllocate(sBuffer);
}

bool GameServer::CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction)
{
	if (direction == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		if (attackerX < targetX && targetX <= (attackerX + AttackRangeX) && abs(attackerY - targetY) <= AttackRangeY)
		{
			return true;
		}
	}
	//LEFT
	else
	{
		if (attackerX - AttackRangeX < targetX && targetX <= attackerX && abs(attackerY - targetY) <= AttackRangeY)
		{
			return true;
		}
	}
	return false;
}

bool GameServer::CheckDirection(char direction)
{
	//��/�찡 �ƴѰ�� ����
	if (direction == static_cast<int>(CHARCTER_DIRECTION_2D::LEFT) || direction == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		return true;
	}
	return false;
}

void GameServer::OnDestroyProc(const SESSION_KEY key)
{
	const auto& iter = _keys.find(key);
	//��ȿ���� ���� ����Ű?
	if (iter == _keys.end())
	{
		DebugBreak();
		return;
	}

	PLAYER_KEY playerKey = iter->second;
	const auto& iter2 = _Players.find(playerKey);
	if (iter2 == _Players.end())
	{
		//��ȿ���� ���� �÷��̾�Ű?
		DebugBreak();
		return;
	}

	Player* DeathPlayer = iter2->second;

#ifdef GAME_DEBUG
	printf("============================================================\n");
	printf("DELETE CHARACTER MESSAGE\n");
	printf("PLAYER ID : %d \n", playerKey);
	printf("============================================================\n");
#endif
	Disconnect(key);
	DeathPlayer->SetPlayerDeath();
}

void GameServer::SendToSector(Common::SerializeBuffer* message, const Player* player)
{
	SECTOR_SURROUND around;
	SECTOR_POS curSector = player->GetSector();

	_pSector->getSurroundSector(curSector.x, curSector.y, around);
	for (int i = 0; i < around._Count; i++)
	{
		//���͸��� ��� Player�鿡�� �ش� �޽��� ������ ��û�ؾ���.
		int targetY = around._Surround[i].y;
		int targetX = around._Surround[i].x;
		for (auto& OtherPlayer : _pSector->_Sector[targetY][targetX])
		{
			if (OtherPlayer->GetPlayerId() == player->GetPlayerId())
			{
				continue;
			}
			SendUniCast(OtherPlayer->GetSessionId(), message, message->getUsedSize());
		}
	}
	
}

void GameServer::cleanUpPlayer()
{
	//��¥ ����� ���
	auto iter = _Players.begin();
	auto iter_e = _Players.end();

	SerializeBuffer* sBuffer = _SbufferPool->allocate();

	for (; iter != iter_e; )
	{
		Player* cur = iter->second;
		int key = iter->first;

		if (cur->IsAlive() == false)
		{
			sBuffer->clear();
			buildMsg_deleteCharacter(static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), key, sBuffer);
			SendBroadCast(cur->GetSessionId(), sBuffer,sBuffer->getUsedSize());

			_keys.erase(cur->GetSessionId());
			_PlayerPool->deAllocate(cur);
			iter = _Players.erase(iter);
			continue;
		}
		++iter;
	}
	_SbufferPool->deAllocate(sBuffer); 
}
//������ ���� 
void GameServer::update()
{
	//�����Ӹ��� �����̱�.
	for (auto& player : _Players)
	{
		Player* cur = player.second;
		if (cur->GetHp() <= 0)
		{
#ifdef GAME_DEBUG
			printf("PLAYER DIE DISCONNECT!\n");
#endif		
			OnDestroyProc(cur->GetSessionId());
			continue;
		}
#ifdef GAME_DEBUG
		//FOR DEBUG
		int prevX = cur->GetX();
		int prevY = cur->GetY();
#endif
		int action = cur->GetAction();
		switch (action)
		{
		case static_cast<int>(MOVE_DIRECTION::LEFT):
			cur->Move(-(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED)), 0);
			break;
		case static_cast<int>(MOVE_DIRECTION::LEFT_TOP):
			cur->Move(-(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED)), -(static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED)));
			break;
		case static_cast<int>(MOVE_DIRECTION::TOP):
			cur->Move(0, -(static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED)));
			break;
		case static_cast<int>(MOVE_DIRECTION::RIGHT_TOP):
			cur->Move(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED), -(static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED)));
			break;
		case static_cast<int>(MOVE_DIRECTION::RIGHT):
			cur->Move(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED), 0);
			break;
		case static_cast<int>(MOVE_DIRECTION::RIGHT_BOTTOM):
			cur->Move(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED), static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED));
			break;
		case static_cast<int>(MOVE_DIRECTION::BOTTOM):
			cur->Move(0, static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED));
			break;
		case static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM):
			cur->Move(-(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED)), static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED));
			break;
		default:
			break;
		}

		if (cur->GetSector() == cur->GetPrevSector())
		{
			continue;
		}
		//���Ϳ� ��ȭ�� �ִٸ�!
		SECTOR_SURROUND deleteArea;
		SECTOR_SURROUND addArea;
		int targetX;
		int targetY;
		_pSector->getUpdateSurroundSector(cur->GetPrevSector(), cur->GetSector(), deleteArea, addArea);

		SerializeBuffer* sBuffer = _SbufferPool->allocate();
		for (int i = 0; i < deleteArea._Count; i++)
		{
			targetX = deleteArea._Surround[i].x;
			targetY = deleteArea._Surround[i].y;

			for (auto& OtherPlayer : _pSector->_Sector[targetY][targetX])
			{
				//delete ������ �ִ� ĳ���鿡�� �� ĳ���� ���� �޽��� ���� 
				sBuffer->clear();
				buildMsg_deleteCharacter(static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), cur->GetPlayerId(), sBuffer);
				SendUniCast(OtherPlayer->GetSessionId(), sBuffer, sBuffer->getUsedSize());

				//������ delete ������ �ִ� ĳ���͵� ���� �޽��� ���� 
				sBuffer->clear();
				buildMsg_deleteCharacter(static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), OtherPlayer->GetPlayerId(), sBuffer);
				SendUniCast(cur->GetSessionId(), sBuffer, sBuffer->getUsedSize());
			}
		}

		for (int i = 0; i < addArea._Count; i++)
		{
			targetX = addArea._Surround[i].x;
			targetY = addArea._Surround[i].y;

			for (auto& OtherPlayer : _pSector->_Sector[targetY][targetX])
			{
				// add ������ �ִ� ĳ���͵鿡�� �� ĳ���� ���� �޽��� ���� (OtherChracter Message)
				sBuffer->clear();
				buildMsg_createOtherCharacter(
					static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
					cur->GetPlayerId(),
					cur->GetDirection(),
					cur->GetX(),
					cur->GetY(),
					cur->GetHp(),
					sBuffer
				);
				SendUniCast(OtherPlayer->GetSessionId(), sBuffer, sBuffer->getUsedSize());

				// ������ add������ �ִ� ĳ���͵� ĳ���� ���� �޽��� ���� (OtherChracter Message)
				sBuffer->clear();
				buildMsg_createOtherCharacter(
					static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
					OtherPlayer->GetPlayerId(),
					OtherPlayer->GetDirection(),
					OtherPlayer->GetX(),
					OtherPlayer->GetY(),
					OtherPlayer->GetHp(),
					sBuffer
				);
				SendUniCast(cur->GetSessionId(), sBuffer, sBuffer->getUsedSize());
			}
		}
		
		
		_SbufferPool->deAllocate(sBuffer);
#ifdef GAME_DEBUG
		int nextX = cur->GetX();
		int nextY = cur->GetY();
		if (prevX == nextX && prevY == nextY)
		{
			continue;
		}
		printf("PLAYER ID : %d | PLAYER X : %hd  |  PLAYER Y : %hd \n", cur->GetPlayerId(), nextX, nextY);
#endif
	}


}
