#pragma once

#include <stdio.h>
#include <sstream>
#include "cmath"
#include <time.h>
#include <Windows.h>
#include "SDL_image.h"
#include "SDL_TTF.h"
#include "SDL_mixer.h"
#include "objects.h"

// Eric
LONGLONG StartCounter(double *);
double GetCounter(ULONGLONG, double);
double FRand(double, double);
double InterpolateLagrangePolynomial(double, double[], double[], int);
void LoadScene(Scene *);
void LoadGame(Scene *);
bool ProcessEvents(Scene *);
void UpdateLogic(Scene *);
bool DirectHitInTheTank(Weapon *, Player);
bool HitInTheTank(Weapon *, Player);
void DoRender(Scene *);
void UpdateRecords(Scene *);
void DestroyScene(Scene *);
void DestroyGame(Scene *);
void LoadRecords(RecordRow[NUMBER_OF_RECORD_ROWS]);
void InitLandscape(Landscape *, Landscape *, int);
void Gravitate(Player[], Landscape);
void DrawLandscape(SDL_Renderer *, Landscape);
void InitPlayers(Player[]);
void DrawTanks(SDL_Renderer *, Player[]);
void RenderWeapon(SDL_Renderer *renderer, Weapon *);

SDL_Texture * LoadTexture(SDL_Renderer *renderer, const char *file);
void LoadTextures(SDL_Renderer *renderer, Player[]);

void PushWeapon(Weapon *, Weapon **, Weapon **);
Weapon * PopWeapon(Weapon **, Weapon **);
void SetHeadOnNext(Weapon **);
void SetHeadOnPrev(Weapon **);

// Hank
void Draw_A_text(Scene *, SDL_Rect, const char *, SDL_Color, SDL_Color, char *, int);
SDL_Texture * CreateTextureFromNumber(SDL_Renderer *, TTF_Font *, int, SDL_Color, SDL_Color);
SDL_Texture * CreateTextureFromText(SDL_Renderer *, TTF_Font *, const char *, SDL_Color, SDL_Color);

void DrawOpeningScreenOfGame(Scene *);
bool DrawScreenChoose_Nbr_Players(Scene *);
void SecondMenuScreenInterations(Scene *, SDL_Rect, SDL_Rect, SDL_Rect);
void CreateAndDraw2PlayersMenu(Scene *);
void DrawBestScoresPanel(Scene *);
void Draw_ALL_BestScoreLines(Scene *, RecordRow[NUMBER_OF_RECORD_ROWS], SDL_Texture *);
void DrawWeaponInsidePanel(Scene *, const char *, int);
int ExitWhileInMenu(Scene *);

void InitTopPanels(Scene *);
void DrawTopPanels(Scene *);
void CreateAndDrawBottomPanels(Scene *, TTF_Font *, Player[]);
void BottomPanelInterations(Player[], int, int, int, Weapon *);

void ObtainNameOfWinner(Scene *, int);
void UpdateAndSaveRecord(Scene *, Player[], int);