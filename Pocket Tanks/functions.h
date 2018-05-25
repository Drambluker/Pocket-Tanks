#pragma once

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include "SDL_image.h"
#include "SDL_TTF.h"
#include "objects.h"

// Eric
LONGLONG StartCounter(double *);
double GetCounter(ULONGLONG, double);
double FRand(double, double);
double InterpolateLagrangePolynomial(double, double[], double[], int);
void LoadScene(Scene *); //
bool ProcessEvents(Scene *); //
void UpdateLogic(Scene *); //
void DoRender(Scene *); //
void DestroyScene(Scene *);
void LoadRecords(RecordRow[NUMBER_OF_RECORD_ROWS]); //
void UpdateRecords(Player[]); //
void InitLandscape(Landscape *, Landscape *, int);
void Gravitate(Player[], Landscape);
void DrawLandscape(SDL_Renderer *, Landscape); //
void InitPlayers(Player[]); //
void DrawTanks(SDL_Renderer *, Player[]); //
void RenderWeapon(SDL_Renderer *renderer, Weapon *); //

SDL_Texture * LoadTexture(SDL_Renderer *renderer, const char *file);
void LoadTextures(SDL_Renderer *renderer, Player[]); //
void DestroyTextures(Player[], Weapon *);

void PushWeapon(Weapon *, Weapon **, Weapon **);
Weapon * PopWeapon(Weapon **);
void SetHeadOnNext(Weapon **);
void SetHeadOnPrev(Weapon **);

// Hank
void InitTopPanels(PlayerTopPanel[]);
void DrawTopPanels(SDL_Renderer *, PlayerTopPanel[], SDL_Texture *, SDL_Texture *, SDL_Texture *, int);
void CreateAndDrawTopPanels(SDL_Renderer *, TTF_Font *, Player[], PlayerTopPanel[]);

void CreateAndDrawBottomPanels(SDL_Renderer *, TTF_Font *, Player[]);
void DrawBottomPanel(SDL_Renderer *, SDL_Texture *, SDL_Texture *, SDL_Texture *, SDL_Texture *, SDL_Texture *, SDL_Texture *, SDL_Texture *, SDL_Texture *, int);
SDL_Texture * CreateTextureFromNumber(SDL_Renderer *, TTF_Font *, int, SDL_Color, SDL_Color);
SDL_Texture * CreateTextureFromText(SDL_Renderer *, TTF_Font *, const char *, SDL_Color, SDL_Color);
void BottomPanelInterations(Player[], int, int, int, Weapon *);
bool GotInTheTank(Weapon *, Player);