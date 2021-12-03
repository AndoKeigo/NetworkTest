#ifndef __MAIN_H__
#define __MAIN_H__

#include "DxLib.h"

//#define _LONLY

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define CELL_WIDTH 50		// �`�掞�̃}�X�̈�ӂ̃s�N�Z����

#define BOARD_WIDTH 8		// �I�Z���Ղ̈�ӂ̃}�X��

#define TARGET_PORT_TCP 4153
#define TARGET_PORT_UDP 4154

enum BoardColor
{
	None = 0,
	Black,
	White
};

enum class GameState
{
	SelectConnect,
	WaitConnect,
	PlayGame
};

typedef struct Vector2Int
{
	int x, y;
}Vector2;

typedef struct MouseData
{
	BOOL isClick;
	int mx, my;
}MouseInfo;

#endif