#pragma once
namespace Core
{
	enum class CHARCTER_DIRECTION_2D
	{
		LEFT = 0,
		RIGHT = 4
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

	enum class PLAYER_MOVE_SPEED
	{
		X_SPEED = 3,
		Y_SPEED = 2,
	};

	enum class PLAYER_DEFAULT
	{
		DEFAULT_ACTION = -1,

		DEFAULT_DIR = 4, //RIGHT
		MOVE_SPEED_X = 3,
		MOVE_SPEED_Y = 2,

		PLAYER_HP = 100,
	};
}
