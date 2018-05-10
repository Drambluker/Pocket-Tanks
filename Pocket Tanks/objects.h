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
	int power;
	Tank tank;
	Weapon *headWeapon, *tailWeapon;
};

struct Landscape
{
	SDL_Point points[SCREEN_WIDTH + 1];
};

struct Landscape2 // Дерево квадрантов
{
	INT8 type;
	SDL_Rect rect;
	Landscape2 *root;
	Landscape2 *quadrants[4];
};

// Hank
struct PlayerTopPanel
{
	SDL_Rect rect;
	Player player;
};

// Eric
struct Scene
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	TTF_Font *font;
	Landscape landscape;
	Player players[2];
	PlayerTopPanel topPanels[2];
};