#pragma once

#include "NetLib.h"
#include "Player.h"
#include "Sector.h"
#include "FrameManager.h"
#include "NetDefine.h"
#include "PlayerDefine.h"
#include <list>
#include <vector>
#include <algorithm>

namespace Core
{
	constexpr int PLAYER_POOL_SIZE = 12500;
	constexpr int PLAYER_RESERVER_SIZE = 12500;

	using namespace NetLib;
	class Player;
	class GameServer : public NetLib::NetWorkLib
	{
		friend class FrameManager;
	public:
		GameServer();
		virtual ~GameServer();

		void registPlayerPool(Common::ObjectPool<Player, PLAYER_POOL_SIZE, false>* pool);
		void registSector(Sector* sector);
		void registFrameManager(FrameManager* frameManager);

		void OnAcceptProc(const SESSION_KEY key) override;
		void OnRecvProc(Common::SerializeBuffer* message, const char msgType, SESSION_KEY key) override;
		void OnDestroyProc(const SESSION_KEY key) override;

		void ReqMoveStartProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqMoveStopProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackLeftHandProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackRightHandProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqAttackKickProc(Common::SerializeBuffer* message, const SESSION_KEY key);
		void ReqEcho(Common::SerializeBuffer* message, const SESSION_KEY key);


		void Post_DeleteSector_DeleteCharacterMsg(const Player* player, const SECTOR_SURROUND& deleteSector);
		void Post_AddSector_CreateCharacterMsg(const Player* player, const SECTOR_SURROUND& addSector);
		void Post_AroundSector_CreateCharacterMsg(const Player* player);
		void Post_AroundSector_DeleteCharacterMsg(const Player* player);
		void Post_AroundSector_MoveStartMsg(const Player* player);
		void Post_AroundSector_MoveStopMsg(const Player* player);
		void Post_AroundSector_LeftHandAttackMsg(const Player* player);
		void Post_AroundSector_RightHandAttackMsg(const Player* player);
		void Post_AroundSector_KickMsg(const Player* player);
		void Post_AroundSector_DamangeMsg(const Player* target, const int attackerId);
		void Post_Me_AroundSector_CreateCharacterMsg(const Player* player);

		void Post_CreateOtherCharacterMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_MoveStartMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_DeleteCharacterMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_MoveStopMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_LeftHandAtkMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_RightHandAtkMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_KickMsg(const Player* sendPlayer, const Player* recvPlayer);
		void Post_DamageMsg(const Player* sendPlayer, const Player* recvPlayer, const int attackerId);


		void CheckAttackSucess(const Player* attacker, Player*& target, const int AttackRangeX, const int AttackRangeY, const SECTOR_SURROUND& attackRangeSector);
		bool CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction);
		bool CheckDirection(char direction);

		void cleanUpPlayer();
		//프레임 로직 
		void update();
		void fixedUpdate();

		//DEBUG
#ifdef GAME_DEUBG
		void printAroundSector()
		{
			const auto& ret = _Players.find(0);
			if (ret != _Players.end())
			{
				SECTOR_SURROUND around;
				Player* target = ret->second;
				SECTOR_POS curSector = target->GetSector();
				std::vector<int> curPlayers;
				curPlayers.reserve(64);
				_pSector->getSurroundSector(curSector.x, curSector.y, around);

				for (int i = 0; i < around._Count; i++)
				{
					int targetY = around._Surround[i].y;
					int targetX = around._Surround[i].x;

					for (auto& AroundPlayer : _pSector->_Sector[targetY][targetX])
					{
						curPlayers.push_back(AroundPlayer->GetPlayerId());
					}
				}
				//주위에 아무것도 없다면, 출력 x
				if (curPlayers.size() == 0)
				{
					return;
				}
				//달라졌다면
				if (curPlayers.size() != playerInSector.size())
				{
					printf("============================================\n");
					for (int i = 0; i < curPlayers.size(); i++)
					{
						printf("PLAYER ID : %d \n", curPlayers[i]);
					}
					printf("============================================\n");
					std::swap(curPlayers, playerInSector);
					return;
				}

				std::sort(curPlayers.begin(), curPlayers.end());
				std::sort(playerInSector.begin(), playerInSector.end());

				for (int i = 0; i < curPlayers.size(); i++)
				{
					//정렬을 했는데, 같지않은게 있다 -> 변화가 생겼다. 
					if (curPlayers[i] != playerInSector[i])
					{
						printf("============================================\n");
						for (int i = 0; i < curPlayers.size(); i++)
						{
							printf("PLAYER ID : %d \n", curPlayers[i]);
						}
						printf("============================================\n");
						std::swap(curPlayers, playerInSector);
						return;
					}
				}

			}
		}
#endif
	private:
		std::unordered_map<NetLib::SESSION_KEY, PLAYER_KEY> _keys;
		std::unordered_map<PLAYER_KEY, Player*> _Players;

		Common::ObjectPool<Player, PLAYER_POOL_SIZE, false>* _PlayerPool;
		FrameManager* _FrameManager;
		Sector* _pSector;
		DWORD _DelayedTime;

		//For Debug 
		std::vector<int> playerInSector;
	};
}