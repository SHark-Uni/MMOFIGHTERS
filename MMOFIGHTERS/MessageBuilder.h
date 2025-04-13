#pragma once

#include "MessageFormat.h"
#include "SerializeBuffer.h"

#define _WINSOCKAPI_
#include <windows.h>
using namespace Common;

namespace Core
{
	inline void buildMsg_createMyCharacter(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y << hp;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y) + sizeof(hp));
		return;
	}
	inline void buildMsg_createOtherCharacter(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y << hp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y) + sizeof(hp));
		return;
	}
	inline void buildMsg_deleteCharacter(_BYTE type, int id, Common::SerializeBuffer* message)
	{
		message->reserveHeader();

		*message << id;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(id));
		return;
	}

	inline void buildMsg_move_start(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y));
		return;
	}

	inline void buildMsg_move_stop(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y));
		return;
	}

	inline void buildMsg_attack_lefthand(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y));
		return;
	}

	inline void buildMsg_attack_righthand(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y));
		return;
	}

	inline void buildMsg_attack_kick(_BYTE type, int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(id) + sizeof(direction) + sizeof(x) + sizeof(y));
		return;
	}

	inline void buildMsg_damage(_BYTE type, int attackId, int targetId, _BYTE targetHp, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << attackId << targetId << targetHp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		message->setHeader(SIGNITURE, type, sizeof(attackId) + sizeof(targetId) + sizeof(targetHp));
		return;
	}

	inline void buildMsg_Sync(_BYTE type, int playerId, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		message->reserveHeader();
		*message << playerId << x << y;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		message->setHeader(SIGNITURE, type, sizeof(playerId) + sizeof(x) + sizeof(y));
		return;
	}
}