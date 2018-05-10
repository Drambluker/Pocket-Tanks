#include "functions.h"
#include "cmath"

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	SDL_Window *window = SDL_CreateWindow("Pocket Tanks",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

	TTF_Font *font = TTF_OpenFont("arial.ttf", 200);
	
	if (!font)
	{
		printf_s("TTF_OpenFont: %s\n", TTF_GetError());
		system("pause");
	}

	int playerLap = 1;

	Landscape landscape;
	InitLandscape(&landscape);

	Player players[2];
	InitPlayers(players);

	LoadTextures(renderer, players);

	Weapon *activeWeapon = NULL;

	PlayerTopPanel topPanels[2];
	InitTopPanels(topPanels);

	SDL_Event event;
	bool quit = false;

	double oldTime = 0, newTime = 0, deltaTime = 0, PCFreq;
	LONGLONG timeStart = StartCounter(&PCFreq);
	// bool mustRedraw = false;
	
	while (!quit)
	{
		newTime = GetCounter(timeStart, PCFreq);
		deltaTime = newTime - oldTime;
		oldTime = newTime;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
			{
				BottomPanelInterations(players, event.button.x, event.button.y, playerLap, activeWeapon);
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
			{
				if (playerLap == 1) players[playerLap - 1].tank.cannon.angle--;
				if (playerLap == 2) players[playerLap - 1].tank.cannon.angle++;
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
			{
				if (playerLap == 1) players[playerLap - 1].tank.cannon.angle++;
				if (playerLap == 2) players[playerLap - 1].tank.cannon.angle--;
			}

			// if sizechanged - mustRedraw = true;

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT)
			{
				players[playerLap - 1].tank.body.rect.x-= 0.03 * deltaTime;
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT)
			{
				players[playerLap - 1].tank.body.rect.x+= 0.06 * deltaTime;
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && activeWeapon == NULL)
			{
				activeWeapon = PopWeapon(&players[playerLap - 1].headWeapon);
				
				if (activeWeapon == NULL && playerLap == 2)
				{
					quit = true;
				}

				if (playerLap == 1) playerLap = 2;
				else playerLap = 1;
			}
		}

		/* Игровая логика */
		SDL_SetRenderDrawColor(renderer, 160, 200, 160, 0);
		SDL_RenderClear(renderer);

		players[0].tank.angle = 180 / Pi * atan((double)(landscape.points[players[0].tank.body.rect.x + players[0].tank.body.rect.w].y -
			landscape.points[players[0].tank.body.rect.x].y) / (landscape.points[players[0].tank.body.rect.x + players[0].tank.body.rect.w].x -
				landscape.points[players[0].tank.body.rect.x].x));
		players[1].tank.angle = 180 / Pi * atan((double)(landscape.points[players[1].tank.body.rect.x + players[1].tank.body.rect.w].y -
			landscape.points[players[1].tank.body.rect.x].y) / (landscape.points[players[1].tank.body.rect.x + players[1].tank.body.rect.w].x -
				landscape.points[players[1].tank.body.rect.x].x));

		Gravitate(players, landscape);

		if (activeWeapon != NULL && GotInTheTank(activeWeapon, players[(playerLap == 2) ? 1 : 0]))
		{
			players[(playerLap == 2) ? 0 : 1].score += activeWeapon->score;
		}

		if (activeWeapon != NULL && (activeWeapon->rect.y >= landscape.points[activeWeapon->rect.x].y || GotInTheTank(activeWeapon, players[(playerLap == 2) ? 1 : 0])))
		{
			SDL_DestroyTexture(activeWeapon->texture);
			activeWeapon->texture = NULL;
			free(activeWeapon);
			activeWeapon = NULL;
		}

		/* Отрисовка сцены */
		DrawLandscape(renderer, landscape);
		DrawTanks(renderer, players);

		RenderWeapon(renderer, activeWeapon);
		
		if (activeWeapon != NULL && activeWeapon->rect.y < landscape.points[activeWeapon->rect.x].y)
		{
			activeWeapon->rect.x += 0.005 * players[(playerLap == 2) ? 0 : 1].power * cos(activeWeapon->angle) * deltaTime; //
			activeWeapon->rect.y += 0.005 * players[(playerLap == 2) ? 0 : 1].power * sin(activeWeapon->angle) * deltaTime + activeWeapon->gravitatin * deltaTime; //
			activeWeapon->gravitatin += 0.0025;
		}

		CreateAndDrawTopPanels(renderer, font, players, topPanels);
		CreateAndDrawBottomPanels(renderer, font, players);

		//CreateAndDrawTopPanels(renderer, font, players, topPanels, mustRedraw); //
		//CreateAndDrawBottomPanels(renderer, font, players, mustRedraw); //
		//mustRedraw = false; //
		
		SDL_RenderPresent(renderer);
	}

	// Освобождение памяти
	Weapon *tempWeapon = NULL;
	for (int i = 0; i < 2; i++)
	{
		tempWeapon = PopWeapon(&players[i].headWeapon);

		while (tempWeapon != NULL)
		{
			free(tempWeapon);
			tempWeapon = NULL;
			tempWeapon = PopWeapon(&players[i].headWeapon);
		}
	}

	DestroyTextures(players, activeWeapon);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}