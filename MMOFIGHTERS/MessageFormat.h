#pragma once

namespace Common
{
	typedef char _BYTE;
	enum class MESSAGE_DEFINE : int
	{
		RES_CREATE_MY_CHARACTER = 0,
		RES_CREATE_OTHER_CHARACTER,
		RES_DELETE_CHARACTER,

		REQ_MOVE_START = 10,
		RES_MOVE_START,
		REQ_MOVE_STOP,
		RES_MOVE_STOP,

		REQ_ATTACK_LEFT_HAND = 20,
		RES_ATTACK_LEFT_HAND,
		REQ_ATTACK_RIGHT_HAND,
		RES_ATTACK_RIGHT_HAND,
		REQ_ATTACK_KICK,
		RES_ATTACK_KICK,

		RES_DAMAGE = 30,

		REQ_SYNC = 250,
		RES_SYNC,
	};
	enum class MOVE_DIRECTION
	{
		LEFT = 0,
		LEFT_TOP = 1,
		TOP = 2,
		RIGHT_TOP = 3,
		RIGHT = 4,
		RIGHT_BOTTOM = 5,
		BOTTOM = 6,
		LEFT_BOTTOM = 7
	};

	/* Client���� ���� MOVE DIR�� ��Ȱ���߱� ������, 0,4�� ���*/
	enum class CHARCTER_DIRECTION_2D
	{
		LEFT = 0,
		RIGHT = 4
	};
	/*
		BYTE	byCode;			// ��Ŷ�ڵ� 0x89 ����.
		BYTE	bySize;			// ��Ŷ ������.
		BYTE	byType;			// ��ŶŸ��.
	*/
#pragma pack(push, 1)
	typedef struct MESSAGE_HEADER
	{
		_BYTE _MessageType;
	}header_t;
#pragma pack(pop)

}