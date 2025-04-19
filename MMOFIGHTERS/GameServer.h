#pragma once

#include "NetLib.h"
#include "Player.h"
#include "Sector.h"
#include "FrameManager.h"
#include <list>

namespace Core
{
	constexpr int PLAYER_POOL_SIZE = 8000;
	constexpr int PLAYER_RESERVER_SIZE = 8000;

	class Player;
	class GameServer : public NetLib::NetWorkLib
	{
	public:
		typedef int PLAYER_KEY;
		GameServer();
		virtual ~GameServer();

		void registPlayerPool(Common::ObjectPool<Player, PLAYER_POOL_SIZE, false>* pool);
		void registSector(Sector* sector);
		void registFrameManager(FrameManager* frameManager);

		void OnAcceptProc(const SESSION_KEY key) override;
		void OnRecvProc(Common::SerializeBuffer* message, const char msgType, SESSION_KEY key) override;
		void OnDestroyProc(const SESSION_KEY key) override;

		void SendToSector(Common::SerializeBuffer* message, const Player* player);
		/*=== 컨텐츠 ====*/
		void ReqMoveStartProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqMoveStopProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackLeftHandProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackRightHandProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackKickProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqEcho(Common::SerializeBuffer* message, const SESSION_KEY key);
		//* 반복되는 코드 모듈로 뺌. 컨텐츠 코드 긴함. *//
		void SendDeleteSectorProc(const Player* player,  const SECTOR_SURROUND& deleteSector);
		void SendAddSectorProc(const Player* player, const SECTOR_SURROUND& addSector);

		void SendCreateMessageToAddSector(const Player* sendPlayer, const Player* recvPlayer, const char msgType);
		void SendMoveStartMessageToAddSector(const Player* sendPlayer, const Player* recvPlayer, const char msgType);

		void CheckAttackSucess(const Player* attacker, Player*& target, const int AttackRangeX, const int AttackRangeY, const SECTOR_SURROUND& attackRangeSector);
		bool CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction);
		bool CheckDirection(char direction);

		void cleanUpPlayer();
		//프레임 로직 
		void update();
		void fixedUpdate();
	private:
		std::unordered_map<SESSION_KEY, PLAYER_KEY> _keys;
		std::unordered_map<PLAYER_KEY, Player*> _Players;

		Common::ObjectPool<Player, PLAYER_POOL_SIZE, false>* _PlayerPool;
		FrameManager* _FrameManager;
		Sector* _pSector;
		DWORD _DelayedTime;
	};
}