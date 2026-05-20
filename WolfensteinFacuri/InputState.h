#pragma once
#include <Windows.h>

struct InputState
{
	UCHAR keys[256]{};
	POINT mousePosition{ 0,0 };
	POINT mouseDelta{ 0,0 };
	bool isMouseLocked{ false };
};