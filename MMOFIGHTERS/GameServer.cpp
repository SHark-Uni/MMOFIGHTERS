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
onAcceptProc에서 할일
1. 플레이어 생성 (Contents코드에서
2. 다른 플레이어에게 내 플레이어 생성 메시지 보내기
3. 기존 플레이어들 나에게 생성 메시지 보내기
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

	//1. 내 캐릭터 생성 메시지 전송 (섹터)
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
	//2.내 캐릭터 생성 메시지 섹터에게 보내주기
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

	//3. 섹터를 돌면서, 섹터에 존재하는 캐릭터들 생성해주는 메시지 보내기.
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

			//움직이고 있다면
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
	//Header 제외하고 payload 넘겨주자
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
		//TODO : 연결 끊어야함.
		printf("BAD REQUEST!\n");
#endif
		OnDestroyProc(key);
		break;
	}
	return;
}
/* 컨텐츠 구현 하기! 예외 케이스들 생각해보자.*/
void GameServer::ReqMoveStartProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char action;
	unsigned short recvX;
	unsigned short recvY;

	*message >> action >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//읽기 실패. 직렬화 버퍼 순서 잘못한거임. 혹은, 메시지 크기가 협의되지 않은상태로 들어옴.
		DebugBreak();
	}

	//범위 넘는 메시지 무시
	if (recvX > RANGE_MOVE_RIGHT || recvY > RANGE_MOVE_BOTTOM)
	{
		return;
	}
	//이상한 방향 무시
	if (action < static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION) || action > static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM))
	{
		return;
	}

	//내 캐릭터 정보 찾기
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;
	player->SetAction(action);
	//방향설정
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
	//나 빼고 다 보내기

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
		//읽기 실패. 직렬화 버퍼 순서 잘못한거임. 혹은, 메시지 크기가 협의되지 않은상태로 들어옴.
		DebugBreak();
	}

	//내 캐릭터 정보 찾기
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

	//무브스탑 메시지 생성 후 보내기
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
		//읽기 실패. 직렬화 버퍼 순서 잘못한거임. 혹은, 메시지 크기가 협의되지 않은상태로 들어옴.
		DebugBreak();
	}
	//이상향 방향이 들어왔다면 무시. (그럴일은 없겠지만)
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

	//어택 Message Send
	buildMsg_attack_lefthand(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), playerKey, attackDir, myX, myY, sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//공격범위 판정
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//내가 내 자신을 때리면 안됨.
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
	//이상향 방향이 들어왔다면 무시. (그럴일은 없겠지만)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//내 캐릭터 정보 찾기
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	short myX = attacker->GetX();
	short myY = attacker->GetY();


	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	//어택 Message Send
	buildMsg_attack_righthand(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_RIGHT_HAND), playerKey, attackDir, myX, myY, sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//공격범위 판정
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//내가 내 자신을 때리면 안됨.
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

	//이상향 방향이 들어왔다면 무시. (그럴일은 없겠지만)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//내 캐릭터 정보 찾기
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	SerializeBuffer* sBuffer = _SbufferPool->allocate();
	sBuffer->clear();

	//어택 Message Send
	buildMsg_attack_kick(static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_KICK), playerKey, attackDir, attacker->GetX(), attacker->GetY(), sBuffer);
	SendBroadCast(key, sBuffer, sBuffer->getUsedSize());

	//공격범위 판정
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
	//좌/우가 아닌경우 무시
	if (direction == static_cast<int>(CHARCTER_DIRECTION_2D::LEFT) || direction == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		return true;
	}
	return false;
}

void GameServer::OnDestroyProc(const SESSION_KEY key)
{
	const auto& iter = _keys.find(key);
	//유효하지 않은 세션키?
	if (iter == _keys.end())
	{
		DebugBreak();
		return;
	}

	PLAYER_KEY playerKey = iter->second;
	const auto& iter2 = _Players.find(playerKey);
	if (iter2 == _Players.end())
	{
		//유효하지 않은 플레이어키?
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
		//섹터마다 모든 Player들에게 해당 메시지 전송을 요청해야함.
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
	//진짜 지우는 경우
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
//프레임 로직 
void GameServer::update()
{
	//프레임마다 움직이기.
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
		//섹터에 변화가 있다면!
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
				//delete 구간에 있는 캐릭들에게 내 캐릭터 삭제 메시지 전송 
				sBuffer->clear();
				buildMsg_deleteCharacter(static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), cur->GetPlayerId(), sBuffer);
				SendUniCast(OtherPlayer->GetSessionId(), sBuffer, sBuffer->getUsedSize());

				//나에게 delete 구간에 있는 캐릭터들 삭제 메시지 전송 
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
				// add 구간에 있는 캐릭터들에게 내 캐릭터 생성 메시지 전송 (OtherChracter Message)
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

				// 나에게 add구간에 있는 캐릭터들 캐릭터 생성 메시지 전송 (OtherChracter Message)
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
