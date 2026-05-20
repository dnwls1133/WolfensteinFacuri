#pragma once
#include "Timer.h"
#include "SceneManager.h"

#define GAME_NAME L"3D Shooting Game"

#define TIMER CGameTimer::GetInstance()

#define SCENE_MANAGER CSceneManager::GetInstance()

enum TextType
{
	TEXT_TITLE,
	TEXT_PRESSSTART,
	TEXT_GAMEOVER,
	TEXT_VICTORY
};