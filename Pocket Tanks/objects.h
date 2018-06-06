#pragma once

#include "SDL.h"
#include "constants.h"

// Eric
struct RecordRow
{
	char name[NAME_LENGTH];
	int score;
};

struct TankCannon
{
	SDL_Rect rect;
	int angle;
	SDL_Texture *texture;
};

struct TankBody
{
	SDL_Rect rect;
	SDL_Texture *texture;
};

struct Tank
{
	TankCannon cannon;
	TankBody body;
	double angle;
};

struct Weapon
{
	char name[NAME_LENGTH];
	int score;
	double angle;
	double gravitatin;
	SDL_Rect rect;
	SDL_Texture *texture;
	SDL_Rect rectOfEffect;
	SDL_Texture *effect;
	Weapon *next;
	Weapon *prev;
};

struct Player
{
	char name[NAME_LENGTH];
	int score;
	int power;
	Tank tank;
	Weapon *headWeapon, *tailWeapon;
	int NbrWeapons;
};

struct Landscape
{
	SDL_Point points[SCREEN_WIDTH + 1];
};

// Hank
struct PlayerTopPanel
{
	SDL_Rect rect;
	Player player;
};

// Common
struct Scene
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	TTF_Font *font;
	SDL_Event event;
	INT8 playerLap;
	Weapon *activeWeapon;
	double oldTime, newTime, deltaTime, PCFreq;
	LONGLONG timeStart;
	int landscapeType;
	Landscape landscape, defaultLandscape;
	Player players[2];
	PlayerTopPanel topPanels[2];
	bool GameOpening, quitSecondScreen, quitThirdScreen;
};