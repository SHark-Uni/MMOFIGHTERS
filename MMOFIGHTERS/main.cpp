#pragma comment(lib, "winmm")
#define _WINSOCKAPI_

#include <iostream>
#include "NetLib.h"
#include "Session.h"
#include "GameServer.h"

using namespace Common;
using namespace NetLib;
using namespace Core;

int main()
{
	::timeBeginPeriod(1);
	DWORD nextTick;
	int sleepTime;

	ObjectPool<Player, PLAYER_POOL_SIZE, false> playerPool;
	ObjectPool<Session, SESSION_POOL_SIZE, false> sessionPool;
	ObjectPool<SerializeBuffer, SBUFFER_POOL_SIZE, false> sBufferPool;

	GameServer* gameServer = new GameServer();
	gameServer->registSessionPool(&sessionPool);
	gameServer->registPlayerPool(&playerPool);
	gameServer->registSBufferPool(&sBufferPool);

	if (gameServer->Init() != eERROR_MESSAGE::SUCCESS)
	{
		return 0;
	}

	nextTick = timeGetTime();
	while (true)
	{
		//네트워크
		gameServer->Process();
		//프레임 로직
		gameServer->update();
		gameServer->cleanUpPlayer();
		gameServer->CleanupSession();

		nextTick += TIME_PER_FRAME;
		sleepTime = nextTick - timeGetTime();
		if (sleepTime > 0)
		{
			Sleep(sleepTime);
		}
		// delayed 타임을 계산해서, delay된 타임이 1프레임이 넘어간다면 update로  따라가기.


	}
	return 0;
}