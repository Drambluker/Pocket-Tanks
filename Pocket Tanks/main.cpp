#include "functions.h"
#include "cmath"

int main(int argc, char** argv)
{
	Scene scene;
	LoadScene(&scene);

	SDL_Event event;
	bool quit = false;
	Weapon *activeWeapon = NULL;
	int playerLap = 1;

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
				BottomPanelInterations(scene.players, event.button.x, event.button.y, playerLap, activeWeapon);
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
			{
				if (playerLap == 1) scene.players[playerLap - 1].tank.cannon.angle--;
				if (playerLap == 2) scene.players[playerLap - 1].tank.cannon.angle++;
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
			{
				if (playerLap == 1) scene.players[playerLap - 1].tank.cannon.angle++;
				if (playerLap == 2) scene.players[playerLap - 1].tank.cannon.angle--;
			}

			// if sizechanged - mustRedraw = true;

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT)
			{
				scene.players[playerLap - 1].tank.body.rect.x-= 0.03 * deltaTime;
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT)
			{
				scene.players[playerLap - 1].tank.body.rect.x+= 0.06 * deltaTime;
			}

			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && activeWeapon == NULL)
			{
				activeWeapon = PopWeapon(&scene.players[playerLap - 1].headWeapon);
				
				if (activeWeapon == NULL && playerLap == 2)
				{
					quit = true;
				}

				if (playerLap == 1) playerLap = 2;
				else playerLap = 1;
			}
		}

		/* Игровая логика */
		SDL_SetRenderDrawColor(scene.renderer, 160, 200, 160, 0);
		SDL_RenderClear(scene.renderer);

		scene.players[0].tank.angle = 180 / Pi * atan((double)(scene.landscape.points[scene.players[0].tank.body.rect.x + scene.players[0].tank.body.rect.w].y -
			scene.landscape.points[scene.players[0].tank.body.rect.x].y) / (scene.landscape.points[scene.players[0].tank.body.rect.x + scene.players[0].tank.body.rect.w].x -
				scene.landscape.points[scene.players[0].tank.body.rect.x].x));
		scene.players[1].tank.angle = 180 / Pi * atan((double)(scene.landscape.points[scene.players[1].tank.body.rect.x + scene.players[1].tank.body.rect.w].y -
			scene.landscape.points[scene.players[1].tank.body.rect.x].y) / (scene.landscape.points[scene.players[1].tank.body.rect.x + scene.players[1].tank.body.rect.w].x -
				scene.landscape.points[scene.players[1].tank.body.rect.x].x));

		Gravitate(scene.players, scene.landscape);

		if (activeWeapon != NULL && GotInTheTank(activeWeapon, scene.players[(playerLap == 2) ? 1 : 0]))
		{
			scene.players[(playerLap == 2) ? 0 : 1].score += activeWeapon->score;
		}

		if (activeWeapon != NULL && (activeWeapon->rect.y >= scene.landscape.points[activeWeapon->rect.x].y || GotInTheTank(activeWeapon, scene.players[(playerLap == 2) ? 1 : 0])))
		{
			SDL_DestroyTexture(activeWeapon->texture);
			activeWeapon->texture = NULL;
			free(activeWeapon);
			activeWeapon = NULL;
		}

		/* Отрисовка сцены */
		DrawLandscape(scene.renderer, scene.landscape);
		DrawTanks(scene.renderer, scene.players);

		RenderWeapon(scene.renderer, activeWeapon);
		
		if (activeWeapon != NULL && activeWeapon->rect.y < scene.landscape.points[activeWeapon->rect.x].y)
		{
			activeWeapon->rect.x += 0.005 * scene.players[(playerLap == 2) ? 0 : 1].power * cos(activeWeapon->angle) * deltaTime; //
			activeWeapon->rect.y += 0.005 * scene.players[(playerLap == 2) ? 0 : 1].power * sin(activeWeapon->angle) * deltaTime + activeWeapon->gravitatin * deltaTime; //
			activeWeapon->gravitatin += 0.0025;
		}

		CreateAndDrawTopPanels(scene.renderer, scene.font, scene.players, scene.topPanels);
		CreateAndDrawBottomPanels(scene.renderer, scene.font, scene.players);

		//CreateAndDrawTopPanels(renderer, font, players, topPanels, mustRedraw); //
		//CreateAndDrawBottomPanels(renderer, font, players, mustRedraw); //
		//mustRedraw = false; //
		
		SDL_RenderPresent(scene.renderer);
	}

	// Освобождение памяти
	Weapon *tempWeapon = NULL;
	for (int i = 0; i < 2; i++)
	{
		tempWeapon = PopWeapon(&scene.players[i].headWeapon);

		while (tempWeapon != NULL)
		{
			free(tempWeapon);
			tempWeapon = NULL;
			tempWeapon = PopWeapon(&scene.players[i].headWeapon);
		}
	}

	DestroyTextures(scene.players, activeWeapon);

	SDL_DestroyRenderer(scene.renderer);
	SDL_DestroyWindow(scene.window);
	TTF_CloseFont(scene.font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}