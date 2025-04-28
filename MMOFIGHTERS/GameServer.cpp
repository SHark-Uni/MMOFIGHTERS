#include "GameServer.h"
#include "MessageFormat.h"
#include "MessageBuilder.h"

#include "PlayerDefine.h"
#include "Logger.h"
#include "NetDefine.h"
#include "ObjectPool.h"
#include "ContentDefine.h"

using namespace NetLib;
using namespace Core;
using namespace Common;


GameServer::GameServer()
{
	_Players.reserve(PLAYER_RESERVER_SIZE);
	_keys.reserve(PLAYER_RESERVER_SIZE);
	_DelayedTime = 0;
}

GameServer::~GameServer()
{

}

void GameServer::registPlayerPool(ObjectPool<Player, PLAYER_POOL_SIZE, false>* pool)
{
	_PlayerPool = pool;
}

void GameServer::registSector(Sector* sector)
{
	_pSector = sector;
}

void GameServer::registFrameManager(FrameManager* frameManager)
{
	_FrameManager = frameManager;
}

void GameServer::OnAcceptProc(const SESSION_KEY key)
{
	int playerKey;
	Player* newPlayer;
	
	newPlayer = _PlayerPool->allocate();
	playerKey = newPlayer->generatePlayerId();
	newPlayer->Init(playerKey, key);

	_pSector->enrollPlayer(newPlayer->GetSector(), newPlayer);
	newPlayer->SetTimeOut(::timeGetTime());
	
	_keys.insert({ key, playerKey });
	_Players.insert({ playerKey, newPlayer });
#ifdef GAME_DEBUG
	SECTOR_POS pos = newPlayer->GetSector();
	printf("========================\n");
	printf("RESPAWN PLAYER\n");
	printf("PLAYER X : %d | PLAYER Y : %d | PLAYER_SECTOR_X : %d | PLAYER_SECTOR_Y : %d \n",
			newPlayer->GetX(),
			newPlayer->GetY(),
			pos.x,
			pos.y
		);
	printf("========================\n");
#endif 
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
	case static_cast<char>(MESSAGE_DEFINE::REQ_MOVE_START):
		ReqMoveStartProc(message, key);
		break;
	case static_cast<char>(MESSAGE_DEFINE::REQ_MOVE_STOP):
		ReqMoveStopProc(message, key);
		break;
	case static_cast<char>(MESSAGE_DEFINE::REQ_ATTACK_LEFT_HAND):
		ReqAttackLeftHandProc(message, key);
		break;
	case static_cast<char>(MESSAGE_DEFINE::REQ_ATTACK_RIGHT_HAND):
		ReqAttackRightHandProc(message, key);
		break;
	case static_cast<char>(MESSAGE_DEFINE::REQ_ATTACK_KICK):
		ReqAttackKickProc(message, key);
		break;
	case static_cast<char>(MESSAGE_DEFINE::REQ_ECHO):
		ReqEcho(message, key);
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
			SendToSector(sBuffer, cur);

			_pSector->dropOutPlayer(cur->GetSector(), cur);
			_keys.erase(cur->GetSessionId());
			_PlayerPool->deAllocate(cur);
			iter = _Players.erase(iter);
			continue;
		}
		++iter;
	}
	_SbufferPool->deAllocate(sBuffer);
}

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
		if (::timeGetTime() - cur->GetTimeOut() >= PLAYER_TIMEOUT)
		{
#ifdef GAME_DEBUG
			printf("PLAYER TIMEOUT DISCONNECT!\n");
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
		if (action != PLAYER_NO_ACTION)
		{
			cur->SetTimeOut(::timeGetTime());
		}
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

		if (cur->IsMoveSector() == false)
		{
			continue;
		}
		//���� �̵��������. ���� ���Ϳ��� ���ְ�, ���� ���Ϳ� ���
		_pSector->dropOutPlayer(cur->GetPrevSector(), cur);
		_pSector->enrollPlayer(cur->GetSector(), cur);
#ifdef GAME_DEBUG
		printf("prevSector : (%d, %d) |  curSector : (%d, %d)| (%d,%d)  ->  (%d, %d) \n", 
			cur->GetPrevSector().y, cur->GetPrevSector().x,
			cur->GetSector().y, cur->GetSector().x,
			cur->GetPrevSector().y, cur->GetPrevSector().x,
			cur->GetSector().y, cur->GetSector().x
		);
#endif
		//���ο� ���͸� ������� Messageó��
		SECTOR_SURROUND deleteArea;
		SECTOR_SURROUND addArea;
		_pSector->getUpdateSurroundSector(cur->GetPrevSector(), cur->GetSector(), deleteArea, addArea);

		SendDeleteSectorProc(cur, deleteArea);
		SendAddSectorProc(cur, addArea);
		
		cur->MoveSectorIsDone();
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
	cleanUpPlayer();
	CleanupSession();
}

void Core::GameServer::fixedUpdate()
{
	update();

	//printAroundSector();
	DWORD deltaTime = _FrameManager->CalculateTimeInterval();
	if (deltaTime < TIME_PER_FRAME)
	{
		Sleep(TIME_PER_FRAME - deltaTime);
	}
	else
	{
		_DelayedTime += (deltaTime - TIME_PER_FRAME);
		if (_DelayedTime >= TIME_PER_FRAME)
		{
			printf("FIXED UPDATE IS EXECUTE FOR %d TIMES \n", (_DelayedTime / TIME_PER_FRAME));
			for (int i = 0; i < (_DelayedTime / TIME_PER_FRAME); i++)
			{
				update(); 
			}
			_DelayedTime %= TIME_PER_FRAME;
			_FrameManager->AddElaspedTime_pastFrame(_DelayedTime / TIME_PER_FRAME);
		}
	}
	_FrameManager->DisplayFrameInfo();
}

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

	//MOVE START �ÿ���, ��ũ�� ��������ұ�? ���߿� �Ѱ������� ���ָ� ���� ������? 

	//���� �Ѵ� �޽��� ����
	if (recvX >= RANGE_MOVE_RIGHT || recvY >= RANGE_MOVE_BOTTOM)
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
	player->SetTimeOut(::timeGetTime());
	player->SetAction(action);
	//���⼳��
	switch (action)
	{
	case static_cast<int>(MOVE_DIRECTION::LEFT):
	case static_cast<int>(MOVE_DIRECTION::LEFT_TOP):
		player->SetDirection(CHARACTER_DIRECTION_LEFT);
		break;
	case static_cast<int>(MOVE_DIRECTION::RIGHT_TOP):
	case static_cast<int>(MOVE_DIRECTION::RIGHT):
	case static_cast<int>(MOVE_DIRECTION::RIGHT_BOTTOM):
		player->SetDirection(CHARACTER_DIRECTION_RIGHT);
		break;
	case static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM):
		player->SetDirection(CHARACTER_DIRECTION_LEFT);
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

	if (recvX >= RANGE_MOVE_RIGHT || recvY >= RANGE_MOVE_BOTTOM)
	{
		return;
	}

	if (CheckDirection(direction) == false)
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;
	player->SetTimeOut(::timeGetTime());

	short playerX = player->GetX();
	short playerY = player->GetY();

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();
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
		buildMsg_Sync(static_cast<char>(MESSAGE_DEFINE::RES_SYNC), playerKey, playerX, playerY, sBuffer);
		SendUniCast(player->GetSessionId(), sBuffer, sBuffer->getUsedSize());
		recvX = playerX;
		recvY = playerY;
		//OnDestroyProc(key);
	}
	//�������� �����.. Client�� ��ǥ�� �Ͼ���.
	player->SetX(recvX);
	player->SetY(recvY);
	player->SetDirection(direction);
	player->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));

	// �ٵ�, �̶� Client�� ��ǥ�� �ϴ°� ������ �����̵��� �Ͼ�ٸ�?  << �̰��� ����� 
	// Ȥ�� �������� ��������, ������ ������ �ߴµ� �����̵��� �ʿ��� ��찡 ���ñ�? << �̰��� ������. ���� Move���� �� ó���� �ߴٸ�
	SECTOR_POS curSector = player->GetSector();
	SECTOR_POS fixedSector = { recvX / SECTOR_WIDTH, recvY / SECTOR_HEIGHT };
	
	//���ͺ�ȭ�� �ִٸ�
	if ((curSector.x != recvX / SECTOR_WIDTH) || (curSector.y != recvY / SECTOR_HEIGHT))
	{
		SECTOR_SURROUND deleteArea;
		SECTOR_SURROUND addArea;
		_pSector->getUpdateSurroundSector(curSector, fixedSector, deleteArea, addArea);

		_pSector->dropOutPlayer(curSector, player);
		_pSector->enrollPlayer(fixedSector, player);

		//���� ���� �������ִ� ���̹Ƿ�, sector������ �������� �ؾ���. 
		player->SetPrevSector(curSector);
		player->SetSector(fixedSector);
		
		//���ͺ�ȭ�� �°� ��Ʈ��ũ �ۼ����� �Ͼ����. 
		SendDeleteSectorProc(player, deleteArea);
		SendAddSectorProc(player, addArea);
	}
#ifdef GAME_DEBUG
	printf("MOVE STOP MESSAGE\n");
	printf("PLAYER ID : %d | SESSION ID : %d | PARAM KEY : %d |CUR_X : %hd  | CUR_Y : %hd |\n", player->GetPlayerId(), player->GetSessionId(), key, player->GetX(), player->GetY());
#endif
	//���꽺ž �޽��� ���� �� ������
	sBuffer->clear();
	buildMsg_move_stop(static_cast<char>(MESSAGE_DEFINE::RES_MOVE_STOP), playerKey, direction, player->GetX(), player->GetY(), sBuffer);
	SendToSector(sBuffer, player);

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
	attacker->SetTimeOut(::timeGetTime());
	Player* target = nullptr;

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	char attackerDirection = attacker->GetDirection();

	sBuffer->clear();
	buildMsg_attack_lefthand(
		static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), 
		playerKey, 
		attackDir, 
		attacker->GetX(),
		attacker->GetY(),
		sBuffer
	);
	SendToSector(sBuffer, attacker);

	SECTOR_SURROUND targetSide;
	SECTOR_POS curSector = attacker->GetSector();
	if (attackerDirection == CHARACTER_DIRECTION_LEFT)
	{
		_pSector->getLeftSideSector(curSector.x, curSector.y, targetSide);
	}
	else
	{
		_pSector->getRightSideSector(curSector.x, curSector.y, targetSide);
	}

	CheckAttackSucess(attacker, target, ATTACK_LEFT_HAND_RANGE_X, ATTACK_LEFT_HAND_RANGE_Y, targetSide);
	if (target != nullptr)
	{
		sBuffer->clear();
		target->Attacked(DAMAGE_LEFT_HAND);
		buildMsg_damage(static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), attacker->GetPlayerId(), target->GetPlayerId(), target->GetHp(), sBuffer);
		SendToSector(sBuffer, target);
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
	Player* target = nullptr;
	attacker->SetTimeOut(::timeGetTime());

	SerializeBuffer* sBuffer = _SbufferPool->allocate();

	char attackerDirection = attacker->GetDirection();

	sBuffer->clear();
	buildMsg_attack_lefthand(
		static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND),
		playerKey,
		attackerDirection,
		attacker->GetX(),
		attacker->GetY(),
		sBuffer
	);
	SendToSector(sBuffer, attacker);
	//���� ����
	SECTOR_SURROUND targetSide;
	SECTOR_POS curSector = attacker->GetSector();
	if (attackerDirection == CHARACTER_DIRECTION_LEFT)
	{
		_pSector->getLeftSideSector(curSector.x, curSector.y, targetSide);
	}
	// ������ �����̸�, ������ ���Ʒ� ���� Ž��
	else
	{
		_pSector->getRightSideSector(curSector.x, curSector.y, targetSide);
	}
	CheckAttackSucess(attacker, target, ATTACK_RIGHT_HAND_RANGE_X, ATTACK_RIGHT_HAND_RANGE_Y, targetSide);
	if (target != nullptr)
	{
		sBuffer->clear();
		target->Attacked(DAMAGE_LEFT_HAND);
		buildMsg_damage(static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), attacker->GetPlayerId(), target->GetPlayerId(), target->GetHp(), sBuffer);
		SendToSector(sBuffer, target);
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

	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//�� ĳ���� ����
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;
	Player* target = nullptr;
	attacker->SetTimeOut(::timeGetTime());

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	char attackerDirection = attacker->GetDirection();

	sBuffer->clear();
	buildMsg_attack_kick(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_KICK), playerKey, attackDir, attacker->GetX(), attacker->GetY(), sBuffer);
	SendToSector(sBuffer, attacker);

	SECTOR_SURROUND targetSide;
	SECTOR_POS curSector = attacker->GetSector();
	if (attackerDirection == CHARACTER_DIRECTION_LEFT)
	{
		_pSector->getLeftSideSector(curSector.x, curSector.y, targetSide);
	}
	// ������ �����̸�, ������ ���Ʒ� ���� Ž��
	else
	{
		_pSector->getRightSideSector(curSector.x, curSector.y, targetSide);
	}
	CheckAttackSucess(attacker, target, ATTACK_KICK_X, ATTACK_KICK_Y, targetSide);
	if (target != nullptr)
	{
		sBuffer->clear();
		target->Attacked(DAMAGE_LEFT_HAND);
		buildMsg_damage(static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), attacker->GetPlayerId(), target->GetPlayerId(), target->GetHp(), sBuffer);
		SendToSector(sBuffer, target);
	}

	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::ReqEcho(Common::SerializeBuffer* message, const SESSION_KEY key)
{
	//������ �״�� �����ֱ�.
	int time;
	*message >> time;
#ifdef GAME_DEBUG
	printf("=================================================\n");
	printf("SERVER ECHO RECEIVED!!\n");
	printf("=================================================\n");
#endif 
	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	buildMsg_Echo(static_cast<char>(MESSAGE_DEFINE::RES_ECHO), time, sBuffer);
	SendUniCast(key, sBuffer, sBuffer->getUsedSize());
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::SendDeleteSectorProc(const Player* player, const SECTOR_SURROUND& deleteSector)
{
	int targetX;
	int targetY;
	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	for (int i = 0; i < deleteSector._Count; i++)
	{
		targetX = deleteSector._Surround[i].x;
		targetY = deleteSector._Surround[i].y;

		for (auto& OtherPlayer : _pSector->_Sector[targetY][targetX])
		{
			//�߰��Ѱ� 
			if (OtherPlayer->GetPlayerId() == player->GetPlayerId())
			{
				continue;
			}
			//delete ������ �ִ� ĳ���鿡�� �� ĳ���� ���� �޽��� ���� 
			sBuffer->clear();
			buildMsg_deleteCharacter(static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), player->GetPlayerId(), sBuffer);
			SendUniCast(OtherPlayer->GetSessionId(), sBuffer, sBuffer->getUsedSize());

			//Debug
			if (OtherPlayer->GetSessionId() == 0)
			{
				printf("DELETE MESSAGE TO 0 FROM %d\n", player->GetPlayerId());
			}
			//������ delete ������ �ִ� ĳ���͵� ���� �޽��� ���� 
			sBuffer->clear();
			buildMsg_deleteCharacter(static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), OtherPlayer->GetPlayerId(), sBuffer);
			SendUniCast(player->GetSessionId(), sBuffer, sBuffer->getUsedSize());
		}
	}

	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::SendAddSectorProc(const Player* player, const SECTOR_SURROUND& addSector)
{
	int targetX;
	int targetY;
	
	for (int i = 0; i < addSector._Count; i++)
	{
		targetX = addSector._Surround[i].x;
		targetY = addSector._Surround[i].y;

		for (auto& OtherPlayer : _pSector->_Sector[targetY][targetX])
		{
			//�߰��Ѱ�
			if (OtherPlayer->GetPlayerId() == player->GetPlayerId())
			{
				continue;
			}
			// add ������ �ִ� ĳ���͵鿡�� �� ĳ���� ���� �޽��� ���� (OtherChracter Message)
			SendCreateMessageToAddSector(player, OtherPlayer, static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER));
			
			//��ȣ�������� �־��.
			if (player->GetAction() != PLAYER_NO_ACTION)
			{
				SendMoveStartMessageToAddSector(player, OtherPlayer, static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START));
			}

			// ������ add������ �ִ� ĳ���͵� ĳ���� ���� �޽��� ���� (OtherChracter Message)
			SendCreateMessageToAddSector(OtherPlayer, player, static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER));
			//�����̰� �ִ� ĳ���� �ִٸ�, ������ �����̶�� �޽��� ����
			if (OtherPlayer->GetAction() != PLAYER_NO_ACTION)
			{
				SendMoveStartMessageToAddSector(OtherPlayer, player, static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START));
			}
		}
	}
	
}

void GameServer::SendCreateMessageToAddSector(const Player* sendPlayer, const Player* recvPlayer, const char msgType)
{
	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();
	buildMsg_createOtherCharacter(
		msgType,
		sendPlayer->GetPlayerId(),
		sendPlayer->GetDirection(),
		sendPlayer->GetX(),
		sendPlayer->GetY(),
		sendPlayer->GetHp(),
		sBuffer
	);

	//Debug
	if (recvPlayer->GetPlayerId() == 0)
	{
		printf("CREATE MESSAGE TO 0 FROM %d\n", sendPlayer->GetPlayerId());
	}

	SendUniCast(recvPlayer->GetSessionId(), sBuffer, sBuffer->getUsedSize());
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::SendMoveStartMessageToAddSector(const Player* sendPlayer, const Player* recvPlayer, const char msgType)
{
	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();
	buildMsg_move_start(
		msgType,
		sendPlayer->GetPlayerId(),
		sendPlayer->GetAction(),
		sendPlayer->GetX(),
		sendPlayer->GetY(),
		sBuffer
	);

	//Debug
	if (recvPlayer->GetPlayerId() == 0)
	{
		printf("MOVE START MESSAGE TO 0 FROM %d\n", sendPlayer->GetPlayerId());
	}

	SendUniCast(recvPlayer->GetSessionId(), sBuffer, sBuffer->getUsedSize());
	_SbufferPool->deAllocate(sBuffer);
}

void GameServer::CheckAttackSucess(const Player* attacker, Player*& target, const int AttackRangeX, const int AttackRangeY, const SECTOR_SURROUND& attackRangeSector)
{
	int nx;
	int ny;

	unsigned short attackerX = attacker->GetX();
	unsigned short attackerY = attacker->GetY();
	char attackerDirection = attacker->GetDirection();

	int dist = -1;
	int distAttackerToTarget;
	for (int i = 0; i < attackRangeSector._Count; i++)
	{
		nx = attackRangeSector._Surround[i].x;
		ny = attackRangeSector._Surround[i].y;
		for (auto AroundPlayer : _pSector->_Sector[ny][nx])
		{
			if (attacker->GetPlayerId() == AroundPlayer->GetPlayerId())
			{
				continue;
			}
			//���ݹ����� �ִ� ���
			if (CheckAttackInRange(
				attackerX, attackerY,
				AttackRangeX, AttackRangeY,
				AroundPlayer->GetX(), AroundPlayer->GetY(), attackerDirection
			))
			{
				//���� ����� Player ã��
				distAttackerToTarget = abs(attackerX - AroundPlayer->GetX()) + abs(attackerY - AroundPlayer->GetY());
				if (dist < distAttackerToTarget)
				{
					dist = distAttackerToTarget;
					target = AroundPlayer;
				}
			}
		}
	}
	return;
}

bool GameServer::CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction)
{
	if (direction == CHARACTER_DIRECTION_RIGHT)
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
	if (direction == CHARACTER_DIRECTION_LEFT || direction == CHARACTER_DIRECTION_RIGHT)
	{
		return true;
	}
	return false;
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
			if ((message->getUsedSize() % 13 == 0) && OtherPlayer->GetPlayerId() == 0)
			{
				int index = message->getUsedSize() / 13;
				for (int i = 0; i < index; i++)
				{
					printf("SESSION ID 0 RECEIEVE MsgType : %d FROM %d\n", *(message->getBufferPtr() + (13 * i + 2)), player->GetPlayerId());
				}

			}
			SendUniCast(OtherPlayer->GetSessionId(), message, message->getUsedSize());
		}
	}
	
}


