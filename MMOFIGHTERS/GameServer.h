#pragma once

#include "NetLib.h"
#include "Player.h"
#include "Sector.h"
#include <list>

namespace Core
{
	constexpr int PLAYER_POOL_SIZE = 6000;
	constexpr int PLAYER_RESERVER_SIZE = 6000;

	class Player;
	class GameServer : public NetLib::NetWorkLib
	{
	public:
		typedef int PLAYER_KEY;
		GameServer();
		virtual ~GameServer();

		void registPlayerPool(Common::ObjectPool<Player, PLAYER_POOL_SIZE, false>* pool);

		void OnAcceptProc(const SESSION_KEY key) override;
		void OnRecvProc(Common::SerializeBuffer* message, const char msgType, SESSION_KEY key) override;
		void OnDestroyProc(const SESSION_KEY key) override;

		void SendToSector(Common::SerializeBuffer* message, const Player* player);
		/*=== ÄÁÅÙÃ÷ ====*/
		void ReqMoveStartProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqMoveStopProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackLeftHandProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackRightHandProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackKickProc(Common::SerializeBuffer* message, const SESSION_KEY key);

		//* ¹Ýº¹µÇ´Â ÄÚµå ¸ðµâ·Î »­. ÄÁÅÙÃ÷ ÄÚµå ±äÇÔ. *//
		void SendDeleteMessage_DeletedSector(const Player* player, Common::SerializeBuffer* sBuffer, const SECTOR_SURROUND& deleteSector);
		void SendCreateMessage_AddSector(const Player* player, Common::SerializeBuffer* sBuffer, const SECTOR_SURROUND& addSector);

		bool CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction);
		bool CheckDirection(char direction);

		void cleanUpPlayer();
		void update();
	private:
		std::unordered_map<SESSION_KEY, PLAYER_KEY> _keys;
		std::unordered_map<PLAYER_KEY, Player*> _Players;

		Common::ObjectPool<Player, PLAYER_POOL_SIZE, false>* _PlayerPool;
		Sector* _pSector;
	};
}