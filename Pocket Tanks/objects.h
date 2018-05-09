#pragma once

#include "SDL.h"
#include "constants.h"

// Eric
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
	const char *name;
	int power; // переместить в Player
	int score;
	double angle;
	double gravitatin; //
	SDL_Rect rect;
	SDL_Texture *texture;
	Weapon *next;
	Weapon *prev;
};

struct Player
{
	const char *name;
	int score;
	Tank tank;
	Weapon *headWeapon, *tailWeapon;
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