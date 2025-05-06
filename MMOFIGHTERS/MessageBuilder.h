#pragma once

#include "MessageFormat.h"
#include "SerializeBuffer.h"

#define _WINSOCKAPI_
#include <windows.h>
using namespace Common;

namespace Core
{
	inline void buildMsg_createMyCharacter(char type, int id, char direction, unsigned short x, unsigned short y, char hp, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y << hp;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y) + sizeof(hp), type);
		return;
	}
	inline void buildMsg_createOtherCharacter(char type, int id, char direction, unsigned short x, unsigned short y, char hp, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y << hp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		message->setHeader(SIGNITURE, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y) + sizeof(hp), type);
		return;
	}
	inline void buildMsg_deleteCharacter(char type, int id, Common::SerializeBuffer* message)
	{
		message->reserveHeader();

		*message << id;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(id), type);
		return;
	}

	inline void buildMsg_move_start(char type, int id, char action, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << action << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(id) + sizeof(action) + sizeof(x) + sizeof(y), type);
		return;
	}

	inline void buildMsg_move_stop(char type, int id, char direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		message->setHeader(SIGNITURE,  sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y), type);
		return;
	}

	inline void buildMsg_attack_lefthand(char type, int id, char direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y), type);
		return;
	}

	inline void buildMsg_attack_righthand(char type, int id, char direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y), type);
		return;
	}

	inline void buildMsg_attack_kick(char type, int id, char direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y), type);
		return;
	}

	inline void buildMsg_damage(char type, int attackId, int targetId, char targetHp, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << attackId << targetId << targetHp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		message->setHeader(SIGNITURE, sizeof(attackId) + sizeof(targetId) + sizeof(targetHp), type);
		return;
	}

	inline void buildMsg_Sync(char type, int playerId, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << playerId << x << y;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(playerId) + sizeof(x) + sizeof(y), type);
		return;
	}

	inline void buildMsg_Echo(char type, int time, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << time;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, sizeof(time), type);
	}
}