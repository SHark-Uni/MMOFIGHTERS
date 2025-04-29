#pragma comment(lib, "winmm")
#define _WINSOCKAPI_

#include <iostream>
#include "NetLib.h"
#include "Session.h"
#include "GameServer.h"
#include "FrameManager.h"

using namespace Common;
using namespace NetLib;
using namespace Core;


int main()
{
	::timeBeginPeriod(1);

	ObjectPool<Player, PLAYER_POOL_SIZE, false>* playerPool = new ObjectPool<Player, PLAYER_POOL_SIZE, false>();
	ObjectPool<Session, SESSION_POOL_SIZE, false>* sessionPool = new ObjectPool<Session, SESSION_POOL_SIZE, false>();
	ObjectPool<SerializeBuffer, SBUFFER_POOL_SIZE, false>* sBufferPool = new ObjectPool<SerializeBuffer, SBUFFER_POOL_SIZE, false>();

	Sector* sector = new Sector();
	FrameManager* frameManager = new FrameManager();

	GameServer* gameServer = new GameServer();
	gameServer->registSessionPool(sessionPool);
	gameServer->registPlayerPool(playerPool);
	gameServer->registSBufferPool(sBufferPool);
	gameServer->registFrameManager(frameManager);
	gameServer->registSector(sector);

	if (gameServer->Init() != eERROR_MESSAGE::SUCCESS)
	{
		return 0;
	}

	int delayedTime = 0;
	frameManager->InitTimer();
	frameManager->InitSector(sector);
	while (true)
	{
		//네트워크
		gameServer->Process();
		//프레임 로직
		gameServer->fixedUpdate();
	}
	return 0;
}