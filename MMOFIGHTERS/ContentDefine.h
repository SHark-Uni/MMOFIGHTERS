#pragma once

namespace Common
{
	constexpr int RANGE_MOVE_TOP = 0;
	constexpr int RANGE_MOVE_LEFT = 0;

	constexpr int MAX_MAP_RANGE = 6400;

	constexpr int RANGE_MOVE_RIGHT = RANGE_MOVE_LEFT + MAX_MAP_RANGE;
	constexpr int RANGE_MOVE_BOTTOM = RANGE_MOVE_TOP + MAX_MAP_RANGE;

	constexpr int MOVE_X_PER_FRAME = 3;
	constexpr int MOVE_Y_PER_FRAME = 2;

	/*SECTOR*/

	constexpr int SECTOR_WIDTH = 64;
	constexpr int SECTOR_HEIGHT = 64;
	constexpr int SECTOR_MAX_COLUMN = MAX_MAP_RANGE / SECTOR_WIDTH;
	constexpr int SECTOR_MAX_ROW = MAX_MAP_RANGE / SECTOR_HEIGHT;

	//3x3 섹터
	constexpr int SECTOR_COLUMN_SIZE = 3;
	constexpr int SECTOR_ROW_SIZE = 3;

	/*좌표 에러 허용 범위*/
	constexpr int COORD_ERROR_TOLERANCE = 50;

	constexpr int ATTACK_LEFT_HAND_RANGE_X = 80;
	constexpr int ATTACK_LEFT_HAND_RANGE_Y = 10;
	constexpr int ATTACK_RIGHT_HAND_RANGE_X = 90;
	constexpr int ATTACK_RIGHT_HAND_RANGE_Y = 10;
	constexpr int ATTACK_KICK_X = 100;
	constexpr int ATTACK_KICK_Y = 20;

	constexpr int DAMAGE_LEFT_HAND = 5;
	constexpr int DAMAGE_RIGHT_HAND = 8;
	constexpr int DAMAGE_KICK = 10;

	constexpr int CHARACTER_DIRECTION_LEFT = 0;
	constexpr int CHARACTER_DIRECTION_RIGHT = 4;
}