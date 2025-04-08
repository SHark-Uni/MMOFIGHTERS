#pragma once

namespace Common
{
	constexpr int RANGE_MOVE_TOP = 50;
	constexpr int RANGE_MOVE_LEFT = 10;
	constexpr int RANGE_MOVE_RIGHT = 630;
	constexpr int RANGE_MOVE_BOTTOM = 470;

	constexpr int MOVE_X_PER_FRAME = 3;
	constexpr int MOVE_Y_PER_FRAME = 2;

	/*좌표 에러 허용 범위*/
	constexpr int COORD_ERROR_TOLERANCE = 50;

	constexpr int ATTACK_LEFT_HAND_RANGE_X = 80;
	constexpr int ATTACK_LEFT_HAND_RANGE_Y = 10;
	constexpr int ATTACK_RIGHT_HAND_RANGE_X = 90;
	constexpr int ATTACK_RIGHT_HAND_RANGE_Y = 10;
	constexpr int ATTACK_KICK_X = 100;
	constexpr int ATTACK_KICK_Y = 20;

	constexpr int DAMAGE_LEFT_HAND = 3;
	constexpr int DAMAGE_RIGHT_HAND = 5;
	constexpr int DAMAGE_KICK = 10;
}