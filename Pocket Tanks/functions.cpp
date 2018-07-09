#include "functions.h"

// Eric
LONGLONG StartCounter(double *PCFreq)
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	*PCFreq = (double)(li.QuadPart) / 1000.0;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

double GetCounter(ULONGLONG CounterStart, double PCFreq)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return (double)(li.QuadPart - CounterStart) / PCFreq;
}

double FRand(double min, double max)
{
	double f = (double)rand() / RAND_MAX;
	return min + f * (max - min);
}

double InterpolateLagrangePolynomial(double x, double xValues[], double yValues[], int size)
{
	double lagrangePol = 0;
	double basicsPol;

	for (int i = 0; i < size; i++)
	{
		basicsPol = 1;

		for (int j = 0; j < size; j++)
		{
			if (j == i) continue;
			basicsPol *= (x - xValues[j]) / (xValues[i] - xValues[j]);
		}

		lagrangePol += basicsPol * yValues[i];
	}

	return lagrangePol;
}

void LoadScene(Scene *scene)
{
	InitLandscape(&scene->landscape, &scene->defaultLandscape, scene->landscapeType);
	InitPlayers(scene->players);
	InitTopPanels(scene);

	LoadTextures(scene->renderer, scene->players);

	scene->activeWeapon = NULL;
	scene->playerLap = 1;
	scene->hitEffect = Mix_LoadWAV("Samples/MGK_Oh_Shit.wav");
}

void LoadGame(Scene *scene)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf_s("SDL_Error: %s\n", SDL_GetError());
		system("pause");
		exit(1);
	}

	if (TTF_Init() < 0)
	{
		printf_s("TTF_Error: %s\n", TTF_GetError());
		system("pause");
		exit(1);
	}

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	scene->window = SDL_CreateWindow("Pocket Tanks", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!scene->window)
	{
		printf_s("SDL_Error: %s\n", SDL_GetError());
		system("pause");
		exit(1);
	}

	//Create vsynced renderer for window
	scene->renderer = SDL_CreateRenderer(scene->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!scene->renderer)
	{
		printf_s("SDL_Error: %s\n", SDL_GetError());
		system("pause");
		exit(1);
	}

	scene->font = TTF_OpenFont("black_ops_one_regular.ttf", 100);
	if (!scene->font)
	{
		printf_s("TTF_Error: %s\n", TTF_GetError());
		system("pause");
		exit(1);
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) < 0)
	{
		printf_s("Mix_Error: %s\n", Mix_GetError());
		system("pause");
		exit(1);
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

	//Working on the Menu.
	DrawOpeningScreenOfGame(scene);

	LoadScene(scene);

	scene->oldTime = scene->newTime = scene->deltaTime = 0;
	scene->timeStart = StartCounter(&scene->PCFreq);
}

bool ProcessEvents(Scene *scene)
{
	scene->newTime = GetCounter(scene->timeStart, scene->PCFreq);
	scene->deltaTime = scene->newTime - scene->oldTime;
	scene->oldTime = scene->newTime;
	double newAngle;
	scene->players[0].scoreTemp = scene->players[0].score;
	scene->players[1].scoreTemp = scene->players[1].score;

	while (SDL_PollEvent(&scene->event)) {
		if (scene->event.type == SDL_QUIT || scene->players[0].headWeapon == NULL && scene->players[1].headWeapon == NULL && scene->activeWeapon == NULL)
			return true;

		if (scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT)
		{
			BottomPanelInterations(scene->players, scene->event.button.x, scene->event.button.y, scene->playerLap, scene->activeWeapon);
		}

		if (scene->event.type == SDL_KEYDOWN && scene->event.key.keysym.sym == SDLK_UP)
		{
			if (scene->playerLap == 1) scene->players[scene->playerLap - 1].tank.cannon.angle--;
			if (scene->playerLap == 2) scene->players[scene->playerLap - 1].tank.cannon.angle++;
		}

		if (scene->event.type == SDL_KEYDOWN && scene->event.key.keysym.sym == SDLK_DOWN)
		{
			if (scene->playerLap == 1) scene->players[scene->playerLap - 1].tank.cannon.angle++;
			if (scene->playerLap == 2) scene->players[scene->playerLap - 1].tank.cannon.angle--;
		}

		// if sizechanged - mustRedraw = true;

		if (scene->event.type == SDL_KEYDOWN && scene->event.key.keysym.sym == SDLK_LEFT && scene->players[scene->playerLap - 1].tank.body.rect.x > 0)
		{
			newAngle = 180 / Pi * atan((double)(scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x - int(0.03 * scene->deltaTime) - 1 + scene->players[scene->playerLap - 1].tank.body.rect.w].y -
				scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x - int(0.03 * scene->deltaTime) - 1].y) / (scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x - int(0.03 * scene->deltaTime) - 1 + scene->players[scene->playerLap - 1].tank.body.rect.w].x -
					scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x - int(0.03 * scene->deltaTime) - 1].x));

			if (newAngle < CRITICAL_ANGLE && newAngle > -45)
				scene->players[scene->playerLap - 1].tank.body.rect.x -= 0.03 * scene->deltaTime;
		}

		if (scene->event.type == SDL_KEYDOWN && scene->event.key.keysym.sym == SDLK_RIGHT && scene->players[scene->playerLap - 1].tank.body.rect.x + scene->players[scene->playerLap - 1].tank.body.rect.w * cos(scene->players[scene->playerLap - 1].tank.angle * Pi / 180) < SCREEN_WIDTH)
		{
			newAngle = 180 / Pi * atan((double)(scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x + int(0.03 * scene->deltaTime) + 1 + scene->players[scene->playerLap - 1].tank.body.rect.w].y -
				scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x + int(0.03 * scene->deltaTime) + 1].y) / (scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x + int(0.03 * scene->deltaTime) + 1 + scene->players[scene->playerLap - 1].tank.body.rect.w].x -
					scene->landscape.points[scene->players[scene->playerLap - 1].tank.body.rect.x + int(0.03 * scene->deltaTime) + 1].x));

			if (newAngle > -CRITICAL_ANGLE && newAngle < 45)
				scene->players[scene->playerLap - 1].tank.body.rect.x += 0.06 * scene->deltaTime;
		}

		if (scene->event.type == SDL_KEYDOWN && scene->event.key.keysym.sym == SDLK_SPACE && scene->activeWeapon == NULL)
		{
			scene->activeWeapon = PopWeapon(&scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);

			if (scene->playerLap == 1) scene->playerLap = 2;
			else scene->playerLap = 1;
		}
	}

	return false;
}

void UpdateLogic(Scene *scene)
{
	SDL_SetRenderDrawColor(scene->renderer, 160, 200, 160, 0);
	// I moved SDL_RenderClear(scene->renderer); from here to the biginning of DoRender();
	SDL_RenderClear(scene->renderer);

	if (scene->players[0].tank.body.rect.x + scene->players[0].tank.body.rect.w < SCREEN_WIDTH)
	{
		scene->players[0].tank.angle = 180 / Pi * atan((double)(scene->landscape.points[int(scene->players[0].tank.body.rect.x + cos(scene->players[0].tank.angle * Pi / 180) * (scene->players[0].tank.body.rect.w - TANK_TURN_ERROR))].y -
			scene->landscape.points[scene->players[0].tank.body.rect.x].y) / (scene->landscape.points[int(scene->players[0].tank.body.rect.x + cos(scene->players[0].tank.angle * Pi / 180) * (scene->players[0].tank.body.rect.w - TANK_TURN_ERROR))].x -
				scene->landscape.points[scene->players[0].tank.body.rect.x].x));
	}

	if (scene->players[1].tank.body.rect.x + scene->players[1].tank.body.rect.w < SCREEN_WIDTH)
	{
		scene->players[1].tank.angle = 180 / Pi * atan((double)(scene->landscape.points[int(scene->players[1].tank.body.rect.x + cos(scene->players[1].tank.angle * Pi / 180) * (scene->players[1].tank.body.rect.w - TANK_TURN_ERROR))].y -
			scene->landscape.points[scene->players[1].tank.body.rect.x].y) / (scene->landscape.points[int(scene->players[1].tank.body.rect.x + cos(scene->players[1].tank.angle * Pi / 180) * (scene->players[1].tank.body.rect.w - TANK_TURN_ERROR))].x -
				scene->landscape.points[scene->players[1].tank.body.rect.x].x));
	}

	Gravitate(scene->players, scene->landscape);

	if (scene->activeWeapon != NULL && (int(scene->activeWeapon->rectOfEffect.w + 0.05 * scene->deltaTime) > 2.25 * scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE && HitInTheTank(scene->activeWeapon, scene->players[(scene->playerLap == 2) ? 1 : 0]) || DirectHitInTheTank(scene->activeWeapon, scene->players[(scene->playerLap == 2) ? 1 : 0])))
	{
		scene->players[(scene->playerLap == 2) ? 0 : 1].score += scene->activeWeapon->score;
		Mix_PlayChannel(-1, scene->hitEffect, 0);
	}
	else if (scene->activeWeapon != NULL && int(scene->activeWeapon->rectOfEffect.w + 0.05 * scene->deltaTime) > 2.25 * scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE && HitInTheTank(scene->activeWeapon, scene->players[(scene->playerLap == 2) ? 0 : 1]))
	{
		scene->players[(scene->playerLap == 2) ? 0 : 1].score -= scene->activeWeapon->score;
		Mix_PlayChannel(-1, scene->hitEffect, 0);
	}

	if (scene->activeWeapon != NULL && (scene->activeWeapon->rect.y >= scene->landscape.points[scene->activeWeapon->rect.x].y ||
		DirectHitInTheTank(scene->activeWeapon, scene->players[(scene->playerLap == 2) ? 1 : 0]) ||
		scene->activeWeapon->rect.x <= 0 ||
		scene->activeWeapon->rect.x >= SCREEN_WIDTH ||
		(scene->activeWeapon->rect.y <= 0 && strcmp(scene->activeWeapon->name, "Laser") == 0) ||
		scene->activeWeapon->rect.y >= SCREEN_HEIGHT)) //
	{
		if (!(DirectHitInTheTank(scene->activeWeapon, scene->players[(scene->playerLap == 2) ? 1 : 0]) ||
			scene->activeWeapon->rect.x <= 0 ||
			scene->activeWeapon->rect.x >= SCREEN_WIDTH ||
			(scene->activeWeapon->rect.y <= 0 && strcmp(scene->activeWeapon->name, "Laser") == 0) ||
			scene->activeWeapon->rect.y >= SCREEN_HEIGHT))
		{
			SDL_Point depthСoordinate = { scene->activeWeapon->rect.x, scene->activeWeapon->rect.y };
			scene->activeWeapon->rectOfEffect = { int(depthСoordinate.x - scene->activeWeapon->rectOfEffect.w / 2), depthСoordinate.y - scene->activeWeapon->rectOfEffect.h / 2, int(scene->activeWeapon->rectOfEffect.w + 0.2 * scene->deltaTime), int(scene->activeWeapon->rectOfEffect.h + 0.2 * scene->deltaTime) };

			if (scene->activeWeapon->rectOfEffect.w > 2.25 * scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE || strcmp(scene->activeWeapon->name, "Laser") == 0)
			{
				if (strcmp(scene->activeWeapon->name, "Chinese Wall") == 0)
				{
					if (!(depthСoordinate.x - 5 >= scene->players[0].tank.body.rect.x && depthСoordinate.x - 5 <= scene->players[0].tank.body.rect.x + scene->players[0].tank.body.rect.w ||
						depthСoordinate.x + 5 >= scene->players[0].tank.body.rect.x && depthСoordinate.x + 5 <= scene->players[0].tank.body.rect.x + scene->players[0].tank.body.rect.w ||
						depthСoordinate.x - 5 >= scene->players[1].tank.body.rect.x && depthСoordinate.x - 5 <= scene->players[1].tank.body.rect.x + scene->players[1].tank.body.rect.w ||
						depthСoordinate.x + 5 >= scene->players[1].tank.body.rect.x && depthСoordinate.x + 5 <= scene->players[1].tank.body.rect.x + scene->players[1].tank.body.rect.w))
					{
						for (int i = depthСoordinate.x - 5; i < depthСoordinate.x + 5; i++)
							if (i >= 0 && i <= SCREEN_WIDTH) scene->landscape.points[i].y -= 300;
					}
				}
				else if (strcmp(scene->activeWeapon->name, "Ravine") == 0)
				{
					for (int i = depthСoordinate.x - 50; i < depthСoordinate.x + 50; i++)
						if (i >= 0 && i <= SCREEN_WIDTH &&
							!(i >= scene->players[0].tank.body.rect.x &&
								i <= scene->players[0].tank.body.rect.x + scene->players[0].tank.body.rect.w ||
								i >= scene->players[1].tank.body.rect.x &&
								i <= scene->players[1].tank.body.rect.x + scene->players[0].tank.body.rect.w)) scene->landscape.points[i].y += 0.8 * SCREEN_HEIGHT - scene->landscape.points[i].y;
				}
				else if (strcmp(scene->activeWeapon->name, "Laser") != 0)
				{
					double t = 0;

					for (int i = depthСoordinate.x - scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE; i <= depthСoordinate.x + scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE; i++)
					{
						if (i >= 0 && i <= SCREEN_WIDTH && (scene->landscape.points[depthСoordinate.x].y >= scene->defaultLandscape.points[depthСoordinate.x].y || scene->landscape.points[i].y <= depthСoordinate.y))
							scene->landscape.points[i].y += scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE * sin(t);

						t += Pi / (2 * scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE + 1);
					}
				}

				SDL_DestroyTexture(scene->activeWeapon->texture);
				scene->activeWeapon->texture = NULL;
				SDL_DestroyTexture(scene->activeWeapon->effect);
				scene->activeWeapon->effect = NULL;
				free(scene->activeWeapon);
				scene->activeWeapon = NULL;
			}
		}
		else
		{
			SDL_DestroyTexture(scene->activeWeapon->texture);
			scene->activeWeapon->texture = NULL;
			SDL_DestroyTexture(scene->activeWeapon->effect);
			scene->activeWeapon->effect = NULL;
			free(scene->activeWeapon);
			scene->activeWeapon = NULL;
		}
	}
}

void DoRender(Scene *scene)
{
	RenderWeapon(scene->renderer, scene->activeWeapon);
	DrawLandscape(scene->renderer, scene->landscape);
	DrawTanks(scene->renderer, scene->players);
	
	if (scene->activeWeapon != NULL) SDL_RenderCopy(scene->renderer, scene->activeWeapon->effect, NULL, &scene->activeWeapon->rectOfEffect); // Render effect

	if (scene->activeWeapon != NULL && scene->activeWeapon->rect.y < scene->landscape.points[scene->activeWeapon->rect.x].y)
	{
		scene->activeWeapon->rect.x += 0.005 * scene->players[(scene->playerLap == 2) ? 0 : 1].power * cos(scene->activeWeapon->angle) * scene->deltaTime;

		if (strcmp(scene->activeWeapon->name, "Laser") == 0)
		{
			scene->activeWeapon->rect.y += 0.005 * scene->players[(scene->playerLap == 2) ? 0 : 1].power * sin(scene->activeWeapon->angle) * scene->deltaTime;
		}
		else if (strcmp(scene->activeWeapon->name, "Pineaple") == 0)
		{
			if (scene->activeWeapon->rect.y >= scene->activeWeapon->rect.y + 0.005 * scene->players[(scene->playerLap == 2) ? 0 : 1].power * sin(scene->activeWeapon->angle) * scene->deltaTime + scene->activeWeapon->gravitatin * scene->deltaTime)
			{
				scene->activeWeapon->rect.y += 0.005 * scene->players[(scene->playerLap == 2) ? 0 : 1].power * sin(scene->activeWeapon->angle) * scene->deltaTime + scene->activeWeapon->gravitatin * scene->deltaTime;
				scene->activeWeapon->gravitatin += 0.0015;
			}
			else
			{
				if (scene->playerLap == 2)
					scene->activeWeapon->angle = atan(fabs(scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.y + scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.h / 2 - scene->activeWeapon->rect.y) / fabs(scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.x + scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.w / 2 - scene->activeWeapon->rect.x));
				else
					scene->activeWeapon->angle = Pi - atan(fabs(scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.y + scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.h / 2 - scene->activeWeapon->rect.y) / fabs(scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.x + scene->players[(scene->playerLap == 2) ? 1 : 0].tank.body.rect.w / 2 - scene->activeWeapon->rect.x));

				scene->activeWeapon->rect.y += 0.005 * scene->players[(scene->playerLap == 2) ? 0 : 1].power * sin(scene->activeWeapon->angle) * scene->deltaTime;
			}
		}
		else
		{
			scene->activeWeapon->rect.y += 0.005 * scene->players[(scene->playerLap == 2) ? 0 : 1].power * sin(scene->activeWeapon->angle) * scene->deltaTime + scene->activeWeapon->gravitatin * scene->deltaTime;
			scene->activeWeapon->gravitatin += 0.0025;
		}
	}

	DrawTopPanels(scene);
	CreateAndDrawBottomPanels(scene, scene->font, scene->players);

	//CreateAndDrawTopPanels(renderer, font, players, topPanels, mustRedraw); //
	//CreateAndDrawBottomPanels(renderer, font, players, mustRedraw);		 //
	//mustRedraw = false;													//

	SDL_RenderPresent(scene->renderer);

	if (scene->FirstRenderOfParty == true)
	{
		scene->musique = Mix_LoadMUS("Samples/Sound_throught_The_Game_Process.ogg");
		Mix_PlayMusic(scene->musique, -1);
		scene->FirstRenderOfParty = false;
	}
}

void UpdateRecords(Scene *scene)
{
	int i = -1;

	if ((scene->players[0].score > scene->players[1].score) && (scene->players[0].score > 0))
		i = 0;
	else if ((scene->players[1].score > scene->players[0].score) && (scene->players[1].score > 0))
		i = 1;

	if (i == 0 || i == 1)
	{
		ObtainNameOfWinner(scene, i);
		if (strlen(scene->WinnerName) > 0) UpdateAndSaveRecord(scene, scene->players, i);
	}
}

void DestroyScene(Scene *scene)
{
	if (scene->activeWeapon != NULL)
	{
		SDL_DestroyTexture(scene->activeWeapon->texture);
		scene->activeWeapon->texture = NULL;
		SDL_DestroyTexture(scene->activeWeapon->effect);
		scene->activeWeapon->effect = NULL;
		free(scene->activeWeapon);
		scene->activeWeapon = NULL;
	}

	Weapon *weapon = NULL;

	for (int i = 0; i < 2; i++)
	{
		weapon = PopWeapon(&scene->players[i].headWeapon, &scene->players[i].tailWeapon);

		while (weapon != NULL)
		{
			SDL_DestroyTexture(weapon->texture);
			weapon->texture = NULL;
			SDL_DestroyTexture(weapon->effect);
			weapon->effect = NULL;
			free(weapon);
			weapon = NULL;
			weapon = PopWeapon(&scene->players[i].headWeapon, &scene->players[i].tailWeapon);
		}

		SDL_DestroyTexture(scene->players[i].tank.cannon.texture);
		scene->players[i].tank.cannon.texture = NULL;
		SDL_DestroyTexture(scene->players[i].tank.body.texture);
		scene->players[i].tank.body.texture = NULL;
	}

	for (int i = 0; i <= 1; i++)
	{
		SDL_DestroyTexture(scene->players[i].textureNamePlayer);
		SDL_DestroyTexture(scene->players[i].textureScorePlayer);
		scene->players[i].textureNamePlayer = NULL;
		scene->players[i].textureScorePlayer = NULL;
	}

	SDL_DestroyTexture(scene->texureWordScore);

	Mix_FreeChunk(scene->hitEffect);
	scene->hitEffect = NULL;
	Mix_FreeMusic(scene->musique);
	scene->musique = NULL;
}

void DestroyGame(Scene *scene)
{
	UpdateRecords(scene);
	DestroyScene(scene);

	//Weapon *tempWeapon = NULL;

	//for (int i = 0; i < 2; i++)
	//{
	//	tempWeapon = PopWeapon(&scene->players[i].headWeapon);

	//	while (tempWeapon != NULL)
	//	{
	//		free(tempWeapon);
	//		tempWeapon = NULL;
	//		tempWeapon = PopWeapon(&scene->players[i].headWeapon);
	//	}
	//}

	int frequency = 44100, channels = MIX_DEFAULT_CHANNELS;
	Uint16 format = MIX_DEFAULT_FORMAT;
	int numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
	printf_s("numtimedopened = %d", numtimesopened);
	for (int i = 1; i <= numtimesopened; i++) Mix_CloseAudio();

	Mix_Quit();
	SDL_DestroyRenderer(scene->renderer);
	SDL_DestroyWindow(scene->window);
	TTF_CloseFont(scene->font);
	TTF_Quit();
	SDL_Quit();
}

void LoadRecords(RecordRow records[NUMBER_OF_RECORD_ROWS])
{
	FILE *recordsFile = NULL;
	fopen_s(&recordsFile, "records.pt", "rb");

	if (recordsFile == NULL)
	{
		printf_s("Try to create file...\n");
		fopen_s(&recordsFile, "records.pt", "wb");

		if (recordsFile == NULL)
		{
			printf_s("Error when create file!");
			system("pause");
			exit(1);
		}

		for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
		{
			strcpy_s(records[i].name, NAME_LENGTH, "Empty");
			records[i].score = 0;
		}

		fwrite(records, sizeof(RecordRow), NUMBER_OF_RECORD_ROWS, recordsFile);
		fclose(recordsFile);

		recordsFile = NULL;
		fopen_s(&recordsFile, "records.pt", "rb");
	}

	fread(records, sizeof(RecordRow), NUMBER_OF_RECORD_ROWS, recordsFile);
	fclose(recordsFile);
	recordsFile = NULL;
}

void InitLandscape(Landscape *landscape, Landscape *defaultLandscape, int type)
{
	srand(time(NULL));

	double xValues[5];
	double yValues[5];

	for (int i = 0, t = 0; i < 5; i++, t += SCREEN_WIDTH / 4)
	{
		xValues[i] = t;
		yValues[i] = FRand(SCREEN_HEIGHT / 2 - 38, SCREEN_HEIGHT / 2 + 38);
	}

	for (int i = 0; i <= SCREEN_WIDTH; i++)
	{
		landscape->points[i].x = defaultLandscape->points[i].x = i;

		switch (type)
		{
		case 1:
			landscape->points[i].y = defaultLandscape->points[i].y = 0.00000058923110602294 * i * i * i - 0.00095600225436248687 * i * i + 0.36244377417817474907 * i + 314.64443045660573261557;
			break;
		case 2:
			landscape->points[i].y = defaultLandscape->points[i].y = -0.00000000000028085715 * i * i * i + 0.00045657530219644915 * i * i - 0.58441598309786968457 * i + 389.09097264457795972703;
			break;
		case 3:
			landscape->points[i].y = defaultLandscape->points[i].y = InterpolateLagrangePolynomial(i, xValues, yValues, 5);
			break;
		default:
			landscape->points[i].y = defaultLandscape->points[i].y = SCREEN_HEIGHT / 2;
			break;
		}
	}
}

void Gravitate(Player players[], Landscape landscape)
{
	while (players[0].tank.body.rect.y < landscape.points[players[0].tank.body.rect.x].y -
		players[0].tank.body.rect.h)
	{
		players[0].tank.body.rect.y++;
	}

	while (players[0].tank.body.rect.y > landscape.points[players[0].tank.body.rect.x].y -
		players[0].tank.body.rect.h)
	{
		players[0].tank.body.rect.y--;
	}

	while (players[1].tank.body.rect.y < landscape.points[players[1].tank.body.rect.x].y -
		players[1].tank.body.rect.h)
	{
		players[1].tank.body.rect.y++;
	}

	while (players[1].tank.body.rect.y > landscape.points[players[1].tank.body.rect.x].y -
		players[1].tank.body.rect.h)
	{
		players[1].tank.body.rect.y--;
	}
}

bool DirectHitInTheTank(Weapon *activeWeapon, Player player)
{
	if (activeWeapon->rect.x > player.tank.body.rect.x && activeWeapon->rect.x < player.tank.body.rect.x + player.tank.body.rect.w &&
		activeWeapon->rect.y > player.tank.body.rect.y && activeWeapon->rect.y < player.tank.body.rect.y + player.tank.body.rect.h)
		return true;

	return false;
}

bool HitInTheTank(Weapon *activeWeapon, Player player)
{
	if (player.tank.body.rect.y + player.tank.body.rect.h / 2 >= activeWeapon->rectOfEffect.y &&  player.tank.body.rect.y + player.tank.body.rect.h / 2 <= activeWeapon->rectOfEffect.y + activeWeapon->rectOfEffect.h &&
		player.tank.body.rect.x + player.tank.body.rect.w / 2 >= activeWeapon->rectOfEffect.x && player.tank.body.rect.x + player.tank.body.rect.w / 2 <= activeWeapon->rectOfEffect.x + activeWeapon->rectOfEffect.w)
	{
		return true;
	}

	return false;
}

void DrawLandscape(SDL_Renderer *renderer, Landscape landscape)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderDrawLines(renderer, landscape.points, SCREEN_WIDTH + 1);

	SDL_SetRenderDrawColor(renderer, 50, 115, 50, 0);

	for (int i = 0; i <= SCREEN_WIDTH; i++)
	{
		SDL_RenderDrawLine(renderer, i, landscape.points[i].y, i, SCREEN_HEIGHT);
	}
}

void InitPlayers(Player players[])
{
	Weapon *weapon = NULL;

	for (int i = 0; i < 2; i++)
	{
		players[i].score = 0;
		players[i].power = 50;
		players[i].tank.body.texture = NULL;
		players[i].tank.cannon.texture = NULL;
		players[i].tank.angle = 0;

		for (int j = 0; j < NUMBER_OF_WEAPON; j++)
		{
			weapon = PopWeapon(&players[i].headWeapon, &players[i].tailWeapon);
			weapon->rectOfEffect.w = weapon->rectOfEffect.h = 0;
			PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		}
	}

	// Player 1
	players[0].tank.body.rect = { 10, 125, 75, 45 };
	players[0].tank.cannon.angle = 0;

	// Player 2
	players[1].tank.body.rect = { SCREEN_WIDTH - players[0].tank.body.rect.w - 10, players[0].tank.body.rect.y,
		players[0].tank.body.rect.w, players[0].tank.body.rect.h };
	players[1].tank.cannon.angle = 180;
}

void DrawTanks(SDL_Renderer *renderer, Player players[])
{
	double r;
	SDL_Point pointOfRotation;

	for (int i = 0; i < 2; i++)
	{
		r = sqrt(players[i].tank.body.rect.w * players[i].tank.body.rect.w / 4 + players[i].tank.body.rect.h * players[i].tank.body.rect.h / CANNON_Y_POSITION_COEF);

		players[i].tank.cannon.rect = { int(players[i].tank.body.rect.x + r * cos(-atan((double)players[i].tank.body.rect.h / sqrt(CANNON_Y_POSITION_COEF) / (players[i].tank.body.rect.w / 2)) + Pi * players[i].tank.angle / 180)),
			int(players[i].tank.body.rect.y + players[i].tank.body.rect.h + r * sin(-atan((double)players[i].tank.body.rect.h / sqrt(CANNON_Y_POSITION_COEF) / (players[i].tank.body.rect.w / 2)) + Pi * players[i].tank.angle / 180)),
			CANNON_W * players[i].tank.body.rect.w / BODY_W + 2,
			CANNON_H * players[i].tank.body.rect.h / BODY_H };

		pointOfRotation = { 0, players[i].tank.cannon.rect.h / 2 };
		SDL_RenderCopyEx(renderer, players[i].tank.cannon.texture, NULL, &players[i].tank.cannon.rect,
			players[i].tank.cannon.angle, &pointOfRotation, SDL_FLIP_NONE);

		if (players[i].headWeapon != NULL)
		{
			players[i].headWeapon->rect = { -2 + players[i].tank.cannon.rect.x + (int)(players[i].tank.cannon.rect.w * cos(-players[i].tank.cannon.angle * Pi / 180)),
				-2 + players[i].tank.cannon.rect.y - (int)(players[i].tank.cannon.rect.w * sin(-players[i].tank.cannon.angle * Pi / 180)),
				7, 7 };
			players[i].headWeapon->angle = players[i].tank.cannon.angle * Pi / 180;
		}

		pointOfRotation = { 0, players[i].tank.body.rect.h };
		SDL_RenderCopyEx(renderer, players[i].tank.body.texture, NULL, &players[i].tank.body.rect,
			players[i].tank.angle, &pointOfRotation, SDL_FLIP_NONE);
	}
}

SDL_Texture * LoadTexture(SDL_Renderer *renderer, const char *file)
{
	SDL_Surface *image = IMG_Load(file);
	SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 255, 0, 255));
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);
	return texture;
}

void LoadTextures(SDL_Renderer *renderer, Player players[])
{
	players[0].tank.body.texture = LoadTexture(renderer, "Sprites/body1.bmp");
	players[0].tank.cannon.texture = LoadTexture(renderer, "Sprites/cannon1.bmp");
	players[1].tank.body.texture = LoadTexture(renderer, "Sprites/body2.bmp");
	players[1].tank.cannon.texture = LoadTexture(renderer, "Sprites/cannon1.bmp");

	Weapon *weapon = NULL;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < NUMBER_OF_WEAPON; j++)
		{
			weapon = PopWeapon(&players[i].headWeapon, &players[i].tailWeapon);
			//weapon->texture = LoadTexture(renderer, "Sprites/weapon1.bmp");
			weapon->effect = LoadTexture(renderer, "Sprites/bang.bmp");
			PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		}
	}
}

void RenderWeapon(SDL_Renderer *renderer, Weapon *activeWeapon)
{
	if (activeWeapon != NULL)
	{
		if (strcmp(activeWeapon->name, "Laser") == 0)
		{
			activeWeapon->rect.h = 6;
			activeWeapon->rect.w = 7 * activeWeapon->rect.h;
			SDL_Point pointOfRotation = { 0, activeWeapon->rect.h / 2 };
			SDL_RenderCopyEx(renderer, activeWeapon->texture, NULL, &activeWeapon->rect, activeWeapon->angle * 180 / Pi, &pointOfRotation, SDL_FLIP_NONE);
			return;
		}
		else
			activeWeapon->rect.w = activeWeapon->rect.h = 7;

		SDL_RenderCopy(renderer, activeWeapon->texture, NULL, &activeWeapon->rect);
	}
}

void PushWeapon(Weapon *weapon, Weapon **headWeapons, Weapon **tailWeapons)
{
	if (*headWeapons == NULL)
	{
		*headWeapons = weapon;
		weapon->prev = NULL;
		weapon->next = NULL;
		*tailWeapons = weapon;
		return;
	}

	(*tailWeapons)->next = weapon;
	weapon->next = NULL;
	weapon->prev = *tailWeapons;
	*tailWeapons = weapon;
}

Weapon * PopWeapon(Weapon **headWeapons, Weapon **tailWeapons)
{
	Weapon *tempWeapon = NULL;

	if (*headWeapons != NULL)
	{
		Weapon *weapon = *headWeapons;

		if ((*headWeapons)->next != NULL)
		{
			tempWeapon = *headWeapons;
			*headWeapons = (*headWeapons)->next;
			(*headWeapons)->prev = tempWeapon->prev;

			if ((*headWeapons)->prev != NULL) (*headWeapons)->prev->next = *headWeapons;
		}
		else
		{
			tempWeapon = *headWeapons;
			*headWeapons = (*headWeapons)->prev;

			if (*headWeapons != NULL) (*headWeapons)->next = tempWeapon->next;
			else *tailWeapons = NULL;
		}

		return weapon;
	}

	return NULL;
}

void SetHeadOnNext(Weapon **headWeapons)
{
	if (*headWeapons != NULL && (*headWeapons)->next != NULL)
	{
		*headWeapons = (*headWeapons)->next;
	}
}

void SetHeadOnPrev(Weapon **headWeapons)
{
	if (*headWeapons != NULL && (*headWeapons)->prev != NULL)
	{
		*headWeapons = (*headWeapons)->prev;
	}
}

// Hank
void Draw_A_text(Scene *scene, SDL_Rect background_rect, const char *text, SDL_Color colorFg, SDL_Color colorBg, const char *position, int sizeOfOneChar)
{
	SDL_SetRenderDrawColor(scene->renderer, colorBg.r, colorBg.g, colorBg.b, colorBg.a);
	SDL_RenderFillRect(scene->renderer, &background_rect);
	int textLength = 0;
	SDL_Rect textBox = { 0, 0, 0, 0 };
	// sizeOfOneChar should be 12 for the test
	textLength = strlen(text);
	textBox.y = background_rect.y + 2;
	textBox.w = textLength * sizeOfOneChar - 3;
	textBox.h = background_rect.h - 4;
	if (position == "Left") textBox.x = background_rect.x + 3;
	if (position == "Center") textBox.x = background_rect.x + (int)(background_rect.w / 2.0) - (int)(textBox.w / 2.0);

	SDL_Surface *Surface_text = NULL;
	SDL_Texture *Texture_text = NULL;
	Surface_text = TTF_RenderText_Solid(scene->font, text, colorFg);
	Texture_text = SDL_CreateTextureFromSurface(scene->renderer, Surface_text);
	SDL_FreeSurface(Surface_text);
	SDL_RenderCopy(scene->renderer, Texture_text, NULL, &textBox);
	SDL_DestroyTexture(Texture_text);
}

void Draw_ALL_BestScoreLines(Scene *scene, RecordRow records[NUMBER_OF_RECORD_ROWS], SDL_Texture *TexturePanelBestScores)
{
	SDL_Color colorBlack = { 0, 0, 0, };
	SDL_Color colorScoresBg = { 128, 128, 0, };
	for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
	{
		if (_stricmp(records[i].name, "Empty") != 0)
		{
			SDL_Rect BestScoreRank_rect = { (int)((71.0 / 90)*SCREEN_WIDTH), (int)((14.0 / 55)*SCREEN_HEIGHT) - (int)(SCREEN_HEIGHT / 20.0) + i * ((int)((3.0 / 55)*SCREEN_HEIGHT)), (int)(SCREEN_WIDTH / 48.0), (int)((3.0 / 55)*SCREEN_HEIGHT) };
			std::string s = std::to_string(i + 1);
			s.append(") ");
			Draw_A_text(scene, BestScoreRank_rect, s.c_str(), colorBlack, colorScoresBg, "Center", 8);
			/*SDL_Texture * textureBestScoreRank = CreateTextureFromNumber(scene->renderer, scene->font, i + 1, colorBlack, colorScoresBg);
			SDL_RenderCopy(scene->renderer, textureBestScoreRank, NULL, &BestScoreRank_rect);*/

			SDL_Rect BestScoreName_rect = { BestScoreRank_rect.x + BestScoreRank_rect.w, BestScoreRank_rect.y, (int)((2.0 / 24)*SCREEN_WIDTH), BestScoreRank_rect.h };
			Draw_A_text(scene, BestScoreName_rect, records[i].name, colorBlack, colorScoresBg, "Center", 12);
			/*SDL_Texture * textureBestScoreName = CreateTextureFromText(scene->renderer, scene->font, records[i].name, colorBlack, colorScoresBg);
			SDL_RenderCopy(scene->renderer, textureBestScoreName, NULL, &BestScoreName_rect);*/

			SDL_Rect BestScoreScore_rect = { BestScoreName_rect.x + BestScoreName_rect.w, BestScoreRank_rect.y, BestScoreRank_rect.w, BestScoreRank_rect.h };
			s = std::to_string(records[i].score);
			Draw_A_text(scene, BestScoreScore_rect, s.c_str(), colorBlack, colorScoresBg, "Center", 14);
			/*SDL_Texture * textureBestScoreScore = CreateTextureFromNumber(scene->renderer, scene->font, records[i].score, colorBlack, colorScoresBg);
			SDL_RenderCopy(scene->renderer, textureBestScoreScore, NULL, &BestScoreScore_rect);*/

			/*SDL_DestroyTexture(textureBestScoreRank);
			SDL_DestroyTexture(textureBestScoreName);
			SDL_DestroyTexture(textureBestScoreScore);*/
		}
	}
}

void DrawBestScoresPanel(Scene *scene)
{
	SDL_Color colorScoresBg = { 128, 128, 0 };
	SDL_Color colorGold = { 255, 215, 0 };
	SDL_Texture* TexturePanelBestScores = SDL_CreateTexture(scene->renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, (int)(SCREEN_WIDTH / 8.0), (int)((3.0 / 5)*SCREEN_HEIGHT));
	//Creating and coloring the Best Score Panel
	SDL_SetRenderDrawColor(scene->renderer, 128, 128, 0, 0);
	SDL_Rect BestScoresPanel_rect = { (int)((71.0 / 90)*SCREEN_WIDTH), (int)((11.0 / 80)*SCREEN_HEIGHT), (int)(SCREEN_WIDTH / 8.0), (int)((3.0 / 5)*SCREEN_HEIGHT) };
	SDL_RenderFillRect(scene->renderer, &BestScoresPanel_rect);
	SDL_SetRenderDrawColor(scene->renderer, 0, 0, 0, 0);
	//Drawing word "Best Scores"
	SDL_Rect WordBestScores_rect = { BestScoresPanel_rect.x, BestScoresPanel_rect.y, BestScoresPanel_rect.w, (int)((3.0 / 55)*SCREEN_HEIGHT) };
	Draw_A_text(scene, WordBestScores_rect, " Best Scores ", colorGold, colorScoresBg, "Center", 10);

	//Prepare and Draw the best scores
	RecordRow records[NUMBER_OF_RECORD_ROWS];
	LoadRecords(records);

	Draw_ALL_BestScoreLines(scene, records, TexturePanelBestScores);

	SDL_SetRenderTarget(scene->renderer, TexturePanelBestScores);
	SDL_SetRenderTarget(scene->renderer, NULL);
	SDL_DestroyTexture(TexturePanelBestScores);
}

void DrawOpeningScreenOfGame(Scene *scene) {
	SDL_Color colorBlack = { 0, 0, 0 };
	SDL_Color colorGrey = { 61, 61, 92 };
	SDL_Color colorFirstScreenBg = { 128, 128, 0 };
	SDL_SetRenderDrawColor(scene->renderer, 128, 128, 0, 0);
	SDL_RenderClear(scene->renderer);
	SDL_Texture* TextureOpenGameScreen = SDL_CreateTexture(scene->renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_Rect WordPocketTanks_rect = { (int)(SCREEN_WIDTH / 10.0), (int)((1.0 / 7)*SCREEN_HEIGHT), (int)((8.0 / 10)*SCREEN_WIDTH), (int)(SCREEN_HEIGHT / 7.0) };
	SDL_Rect WordBy_rect = { (int)(SCREEN_WIDTH / 2) - 12, SCREEN_HEIGHT - 169, 40, 42 };
	SDL_Rect Name_Hank_rect = { (int)(SCREEN_WIDTH / 10.0), SCREEN_HEIGHT - 77 , 364, 42 };
	SDL_Rect Contact_Hank_rect = { Name_Hank_rect.x, Name_Hank_rect.y + Name_Hank_rect.h , Name_Hank_rect.w, Name_Hank_rect.h };
	SDL_Rect Name_Eric_rect = { (int)((9.0 / 10)*SCREEN_WIDTH) - 285, Name_Hank_rect.y, 285, Name_Hank_rect.h };
	SDL_Rect Contact_Eric_rect = { Name_Eric_rect.x, Name_Eric_rect.y + Name_Eric_rect.h , Name_Eric_rect.w,Name_Eric_rect.h };

	SDL_Texture * textureWordPocketTanks = LoadTexture(scene->renderer, "Sprites/Logo_Pocket_Tanks.BMP");

	SDL_RenderCopy(scene->renderer, textureWordPocketTanks, NULL, &WordPocketTanks_rect);
	Draw_A_text(scene, WordBy_rect, " By ", colorBlack, colorFirstScreenBg, "Center", 12);
	Draw_A_text(scene, Name_Hank_rect, "14karra (Hank Djambong)", colorBlack, colorFirstScreenBg, "Center", 12);
	Draw_A_text(scene, Contact_Hank_rect, "Twitter@HDjambong", colorBlack, colorFirstScreenBg, "Center", 12);
	Draw_A_text(scene, Name_Eric_rect, "Drambluker (Eric Vlaskin)", colorBlack, colorFirstScreenBg, "Center", 12);
	Draw_A_text(scene, Contact_Eric_rect, "Twitter@drambluker", colorBlack, colorFirstScreenBg, "Center", 12);

	//Destroy Textures
	SDL_DestroyTexture(textureWordPocketTanks);
	SDL_SetRenderTarget(scene->renderer, TextureOpenGameScreen);
	SDL_RenderPresent(scene->renderer);
	SDL_SetRenderTarget(scene->renderer, NULL);
	SDL_DestroyTexture(TextureOpenGameScreen);
	time_t enter_time, finishFirstScreen_time;
	time(&enter_time);

	ShowCursor(false);
	do {
		time(&finishFirstScreen_time);
	} while (difftime(finishFirstScreen_time, enter_time) < 4);
	ShowCursor(true);

	DrawScreenChoose_Nbr_Players(scene);
}

bool DrawScreenChoose_Nbr_Players(Scene *scene)
{
	scene->musique = Mix_LoadMUS("Samples/Sound_in_the_Menu.ogg");
	Mix_PlayMusic(scene->musique, -1);
	SDL_Color colorBlack = { 0, 0, 0 };
	SDL_Color colorGrey = { 61, 61, 92 };

	//Prepare to show the second screen
	SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
	SDL_RenderClear(scene->renderer);

	//Draw the main screen
	SDL_Texture* TextureMenuStart = SDL_CreateTexture(scene->renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, (int)((11.0 / 15)*SCREEN_WIDTH), (int)((14.0 / 15)*SCREEN_HEIGHT));

	SDL_Rect Button1Player_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), (int)((17.0 / 60)*SCREEN_HEIGHT), (int)(SCREEN_WIDTH / 6.0), (int)(SCREEN_HEIGHT / 12.0) };
	SDL_Rect Button2Players_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), (int)((17.0 / 60)*SCREEN_HEIGHT) + Button1Player_rect.h + 100, Button1Player_rect.w, Button1Player_rect.h };

	Draw_A_text(scene, Button1Player_rect, " 1 PLAYER ", colorBlack, colorGrey, "Center", 15);
	Draw_A_text(scene, Button2Players_rect, " 2 PLAYERS ", colorBlack, colorGrey, "Center", 15);

	SDL_SetRenderTarget(scene->renderer, TextureMenuStart);
	SDL_SetRenderTarget(scene->renderer, NULL);
	SDL_DestroyTexture(TextureMenuStart);

	//Draw the right panel for Best Scores
	DrawBestScoresPanel(scene);

	//Drawing the button "Quit the Game"
	SDL_Rect ButtonQuitTheGame_rect = { (int)((71.0 / 90)*SCREEN_WIDTH), (int)((14.0 / 15)*SCREEN_HEIGHT), (int)(SCREEN_WIDTH / 8.0), (int)(SCREEN_HEIGHT / 15.0) };
	Draw_A_text(scene, ButtonQuitTheGame_rect, "Quit Game", colorBlack, colorGrey, "Center", 12);

	SDL_RenderPresent(scene->renderer);

	//Initializing the players's names with test values
	strcpy_s(scene->players[0].name, NAME_LENGTH, "Player 1");
	strcpy_s(scene->players[1].name, NAME_LENGTH, "Player 2");


	//Working with interactions on the second screen
	scene->quitSecondScreen = false;
	while (scene->quitSecondScreen != true)
	{
		while (SDL_PollEvent(&scene->event))
		{
			if (scene->event.type == SDL_QUIT)ExitWhileInMenu(scene);
			if (scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT)
			{
				SecondMenuScreenInterations(scene, Button1Player_rect, Button2Players_rect, ButtonQuitTheGame_rect);
				break;
			}
		}
	}
	return true;
}

void SecondMenuScreenInterations(Scene *scene, SDL_Rect Button1Player_rect, SDL_Rect Button2Players_rect, SDL_Rect ButtonQuitTheGame_rect)
{
	if ((scene->event.button.x >= Button1Player_rect.x) && (scene->event.button.x <= Button1Player_rect.x + Button1Player_rect.w)
		&& (scene->event.button.y >= Button1Player_rect.y) && (scene->event.button.y <= Button1Player_rect.y + Button1Player_rect.h))
		//CreateAndDraw1PlayerMenu(scene);
		// NEEDS TO BE DONE !!!
		true;
	if ((scene->event.button.x >= Button2Players_rect.x) && (scene->event.button.x <= Button2Players_rect.x + Button2Players_rect.w)
		&& (scene->event.button.y >= Button2Players_rect.y) && (scene->event.button.y <= Button2Players_rect.y + Button2Players_rect.h))
	{
		CreateAndDraw2PlayersMenu(scene);
	}
	if ((scene->event.button.x >= ButtonQuitTheGame_rect.x) && (scene->event.button.x <= ButtonQuitTheGame_rect.x + ButtonQuitTheGame_rect.w)
		&& (scene->event.button.y >= ButtonQuitTheGame_rect.y) && (scene->event.button.y <= ButtonQuitTheGame_rect.y + ButtonQuitTheGame_rect.h))
		ExitWhileInMenu(scene);
}

void CreateAndDraw2PlayersMenu(Scene *scene)
{
	SDL_Color colorBlack = { 0, 0, 0 };
	SDL_Color colorWhite = { 255, 255, 255 };
	SDL_Color colorGold = { 255, 215, 0 };
	SDL_Color colorGrey = { 61, 61, 92 };
	SDL_Color colorBg = { 153, 153, 0 };
	SDL_Color colorScoresBg = { 128, 128, 0 };
	SDL_Rect MainScreenOfMenu_rect = { (int)(SCREEN_WIDTH / 30.0), 0, (int)((11.0 / 15)*SCREEN_WIDTH), SCREEN_HEIGHT };
	//Repaint all the main screen of menu
	SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
	SDL_RenderFillRect(scene->renderer, &MainScreenOfMenu_rect);
	SDL_Texture* TextureMainMenuStart = SDL_CreateTexture(scene->renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, MainScreenOfMenu_rect.w, MainScreenOfMenu_rect.h);

	//Drawing the main screen of menu
	SDL_Rect WeaponShop_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), (int)(SCREEN_HEIGHT / 80.0), (int)(SCREEN_WIDTH / 8.0) + 100, (int)(SCREEN_HEIGHT / 20.0) + 50 };
	SDL_Texture * textureWeaponShop = LoadTexture(scene->renderer, "Sprites/imgWeaponShop.BMP");
	SDL_RenderCopy(scene->renderer, textureWeaponShop, NULL, &WeaponShop_rect);
	SDL_DestroyTexture(textureWeaponShop);

	SDL_Rect Player1_rect = { (int)(SCREEN_WIDTH / 30.0), (int)((3.0 / 40)*SCREEN_HEIGHT), (int)(SCREEN_WIDTH / 8.0), (int)(SCREEN_HEIGHT / 20.0) };
	SDL_Texture * texturePlayer1 = LoadTexture(scene->renderer, "Sprites/imgPlayer1.BMP");
	SDL_RenderCopy(scene->renderer, texturePlayer1, NULL, &Player1_rect);
	SDL_DestroyTexture(texturePlayer1);

	SDL_Rect Player2_rect = { (int)((77.0 / 120)*SCREEN_WIDTH), Player1_rect.y, Player1_rect.w, Player1_rect.h };
	SDL_Texture * texturePlayer2 = LoadTexture(scene->renderer, "Sprites/imgPlayer2.BMP");
	SDL_RenderCopy(scene->renderer, texturePlayer2, NULL, &Player2_rect);
	SDL_DestroyTexture(texturePlayer2);

	//Draw the two containers(black spaces) for the weapons that would be selected
	SDL_Rect BlancForWeapons1_rect = { (int)(SCREEN_WIDTH / 30.0), (int)((11.0 / 80)*SCREEN_HEIGHT), Player1_rect.w, (int)((3.0 / 5)*SCREEN_HEIGHT) };
	SDL_Rect BlancForWeapons2_rect = { (int)((77.0 / 120)*SCREEN_WIDTH), BlancForWeapons1_rect.y, BlancForWeapons1_rect.w, BlancForWeapons1_rect.h };
	SDL_SetRenderDrawColor(scene->renderer, 0, 0, 0, 0);
	SDL_RenderDrawRect(scene->renderer, &BlancForWeapons1_rect);
	SDL_RenderDrawRect(scene->renderer, &BlancForWeapons2_rect);

	//Draw the Weapons to the center of the screen for their selection
	SDL_Rect ChooseWeapons_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), BlancForWeapons1_rect.y, (int)(SCREEN_WIDTH / 7.0), (int)((3.0 / 55)*SCREEN_HEIGHT) };
	Draw_A_text(scene, ChooseWeapons_rect, "Choose Weapons:", colorBlack, colorBg, "Center", 12);

	//Draw landscapes
	SDL_Rect WordLandscape_rect = { (int)(SCREEN_WIDTH / 30.0), (int)((4.0 / 5)*SCREEN_HEIGHT) - (int)(SCREEN_HEIGHT / 20.0) + 10, (int)(SCREEN_WIDTH / 8.0), (int)(SCREEN_HEIGHT / 15.0) - 10 };
	SDL_Texture * textureWordLandscape = LoadTexture(scene->renderer, "Sprites/imgLandscape.BMP");
	SDL_RenderCopy(scene->renderer, textureWordLandscape, NULL, &WordLandscape_rect);
	SDL_DestroyTexture(textureWordLandscape);

	SDL_Rect Landscape1_rect = { (int)((37.0 / 192)*(SCREEN_WIDTH)), (int)((4.0 / 5)*SCREEN_HEIGHT) - (int)(SCREEN_HEIGHT / 20.0) + 10, (int)((11.0 / 80)*SCREEN_WIDTH) - 70, (int)(SCREEN_HEIGHT / 15.0) - 10 };
	SDL_Rect Landscape2_rect = { Landscape1_rect.x + Landscape1_rect.w + (int)((11.0 / 320)*(SCREEN_WIDTH)), Landscape1_rect.y, Landscape1_rect.w, Landscape1_rect.h };
	SDL_Rect Landscape3_rect = { Landscape2_rect.x + Landscape2_rect.w + (int)((11.0 / 320)*(SCREEN_WIDTH)), Landscape2_rect.y, Landscape2_rect.w, Landscape2_rect.h };
	SDL_Rect Landscape4_rect = { Landscape3_rect.x + Landscape3_rect.w + (int)((11.0 / 320)*(SCREEN_WIDTH)), Landscape3_rect.y, Landscape3_rect.w, Landscape3_rect.h };

	SDL_Texture * textureLandscape1 = LoadTexture(scene->renderer, "Sprites/imgLandscape1.BMP");
	SDL_Texture * textureLandscape2 = LoadTexture(scene->renderer, "Sprites/imgLandscape2.BMP");
	SDL_Texture * textureLandscape3 = LoadTexture(scene->renderer, "Sprites/imgLandscape3.BMP");
	SDL_Texture * textureLandscape4 = LoadTexture(scene->renderer, "Sprites/imgLandscape4.BMP");

	SDL_RenderCopy(scene->renderer, textureLandscape1, NULL, &Landscape1_rect);
	SDL_RenderCopy(scene->renderer, textureLandscape2, NULL, &Landscape2_rect);
	SDL_RenderCopy(scene->renderer, textureLandscape3, NULL, &Landscape3_rect);
	SDL_RenderCopy(scene->renderer, textureLandscape4, NULL, &Landscape4_rect);

	SDL_DestroyTexture(textureLandscape1);
	SDL_DestroyTexture(textureLandscape2);
	SDL_DestroyTexture(textureLandscape3);
	SDL_DestroyTexture(textureLandscape4);

	//Draw Button "Restart"
	SDL_Color colorButtonRestart = { 128, 128, 0 };
	SDL_Rect ButtonRestart_rect = { BlancForWeapons1_rect.x, (int)((13.0 / 15)*SCREEN_HEIGHT), (int)(SCREEN_WIDTH / 16.0), (int)(SCREEN_HEIGHT / 15.0) };
	Draw_A_text(scene, ButtonRestart_rect, "Restart", colorBlack, colorButtonRestart, "Center", 8);

	//Draw Button "Randomly"

	SDL_Rect ButtonRandomly_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), ButtonRestart_rect.y, (int)(SCREEN_WIDTH / 7.0), ButtonRestart_rect.h };
	Draw_A_text(scene, ButtonRandomly_rect, "Randomly", colorBlack, colorScoresBg, "Center", 12);

	//Draw Button "Play"
	SDL_Color colorButtonPlay = { 153, 153, 100 };
	SDL_Rect ButtonPlay_rect = { BlancForWeapons2_rect.x + BlancForWeapons2_rect.w - ButtonRestart_rect.w, ButtonRestart_rect.y, ButtonRestart_rect.w, ButtonRestart_rect.h };
	Draw_A_text(scene, ButtonPlay_rect, "Play", colorBlack, colorButtonPlay, "Center", 12);

	//Draw Weapons 
	SDL_Rect LollyBomb_rect = { (int)((11.0 / 40)*SCREEN_WIDTH), ChooseWeapons_rect.y + ChooseWeapons_rect.h, (int)(SCREEN_WIDTH / 8.0) - 10, (int)((3.0 / 55)*SCREEN_HEIGHT) - 10 };
	SDL_Rect Laser_rect = { (int)((40.0 / 100)*SCREEN_WIDTH) + 10, LollyBomb_rect.y, LollyBomb_rect.w, LollyBomb_rect.h };
	SDL_Rect Ravine_rect = { LollyBomb_rect.x, LollyBomb_rect.y + LollyBomb_rect.h + 10, LollyBomb_rect.w, LollyBomb_rect.h };
	SDL_Rect LollyBomb2_rect = { Laser_rect.x, Laser_rect.y + Laser_rect.h + 10, Laser_rect.w, Laser_rect.h };
	SDL_Rect ChineseWall_rect = { LollyBomb_rect.x, Ravine_rect.y + Ravine_rect.h + 10, LollyBomb_rect.w, LollyBomb_rect.h };
	SDL_Rect Pineaple_rect = { Laser_rect.x, LollyBomb2_rect.y + LollyBomb2_rect.h + 10, Laser_rect.w, Laser_rect.h };

	clock_t Begin_time = clock();
	clock_t End_time;
	do {
		End_time = clock();
	} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.3);
	Draw_A_text(scene, LollyBomb_rect, "Lolly Bomb", colorBlack, colorScoresBg, "Center", 9);
	SDL_RenderPresent(scene->renderer);

	Begin_time = clock();
	do {
		End_time = clock();
	} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.3);
	Draw_A_text(scene, Laser_rect, "Laser", colorBlack, colorScoresBg, "Center", 9);
	SDL_RenderPresent(scene->renderer);

	Begin_time = clock();
	do {
		End_time = clock();
	} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.3);
	Draw_A_text(scene, Ravine_rect, "Ravine", colorBlack, colorScoresBg, "Center", 9);
	SDL_RenderPresent(scene->renderer);

	Begin_time = clock();
	do {
		End_time = clock();
	} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.3);
	Draw_A_text(scene, LollyBomb2_rect, "Lolly Bomb 2.0", colorBlack, colorScoresBg, "Center", 8);
	SDL_RenderPresent(scene->renderer);

	Begin_time = clock();
	do {
		End_time = clock();
	} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.3);
	Draw_A_text(scene, ChineseWall_rect, "Chinese Wall", colorBlack, colorScoresBg, "Center", 8);
	SDL_RenderPresent(scene->renderer);

	Begin_time = clock();
	do {
		End_time = clock();
	} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.3);
	Draw_A_text(scene, Pineaple_rect, "Pineaple", colorBlack, colorScoresBg, "Center", 9);
	SDL_RenderPresent(scene->renderer);

	//Interactions on the Third Screen
	SDL_Rect ButtonQuitTheGame_rect = { (int)((71.0 / 90)*SCREEN_WIDTH), (int)((14.0 / 15)*SCREEN_HEIGHT), (int)(SCREEN_WIDTH / 8.0), (int)(SCREEN_HEIGHT / 15.0) };
	scene->quitThirdScreen = false;
	scene->playerLap = 1;
	bool Choosed_LollyBomb = false, Choosed_Laser = false, Choosed_Ravine = false, Choosed_LollyBomb2 = false, Choosed_ChineseWall = false, Choosed_Pineaple = false;
	scene->players[0].headWeapon = NULL;
	scene->players[0].tailWeapon = NULL;
	scene->players[1].headWeapon = NULL;
	scene->players[1].tailWeapon = NULL;

	////Working On The Names
	//
	//Begin_time = clock();
	//do {
	//	End_time = clock();
	//} while ((float(End_time - Begin_time) / CLOCKS_PER_SEC) <= 0.5);
	//bool NamesEntered = false, quit = false;
	//const char *Msg1;
	//Msg1 = "Enter Name Player 1";
	//DrawMsgOnBottonScreen(scene, Msg1);
	//SDL_RenderPresent(scene->renderer);
	///*
	//while (quit != true)
	//{
	//	while (SDL_PollEvent(&scene->event))
	//	{
	//		if (scene->event.type == SDL_QUIT)ExitWhileInMenu(scene);
	//		if (scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT)
	//		{
	//			//Event for button "Quit The Game"
	//			if ((scene->event.button.x >= ButtonQuitTheGame_rect.x) && (scene->event.button.x <= ButtonQuitTheGame_rect.x + ButtonQuitTheGame_rect.w)
	//				&& (scene->event.button.y >= ButtonQuitTheGame_rect.y) && (scene->event.button.y <= ButtonQuitTheGame_rect.y + ButtonQuitTheGame_rect.h))
	//				ExitWhileInMenu(scene);
	//			//Events for reading and printing the name 1
	//			if ((scene->event.button.x >= Blanc1_rect.x) && (scene->event.button.x <= Blanc1_rect.x + Blanc1_rect.w)
	//				&& (scene->event.button.y >= Blanc1_rect.y) && (scene->event.button.y <= Blanc1_rect.y + Blanc1_rect.h))
	//			{
	//				WorkingOnName(scene, &Blanc1_rect, 0);
	//				quit = true;
	//				break;
	//			}
	//		}
	//	}
	//}
	//*/
	//NamesEntered = true;

	//Event for Choosing Weapons
	scene->players[0].NbrWeapons = 0;
	scene->players[1].NbrWeapons = 0;
	SDL_Rect WordCurrentPlayerChoosing_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), (int)((14.0 / 15)*SCREEN_HEIGHT) + 10, (int)(SCREEN_WIDTH / 7.0), (int)(SCREEN_HEIGHT / 15.0) - 10 };
	Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Weapons...", colorBlack, colorBg, "Center", 8);
	SDL_RenderPresent(scene->renderer);
	scene->playerLap = 1;
	bool WeaponsChoosed = false, Selected_A_Weapon = false;
	while (WeaponsChoosed != true)
	{

		while (SDL_PollEvent(&scene->event))
		{
			if (scene->event.type == SDL_QUIT)ExitWhileInMenu(scene);
			if (scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT)
			{
				//Event for button "Quit The Game"
				if ((scene->event.button.x >= ButtonQuitTheGame_rect.x) && (scene->event.button.x <= ButtonQuitTheGame_rect.x + ButtonQuitTheGame_rect.w)
					&& (scene->event.button.y >= ButtonQuitTheGame_rect.y) && (scene->event.button.y <= ButtonQuitTheGame_rect.y + ButtonQuitTheGame_rect.h))
					ExitWhileInMenu(scene);
				////Event for the Weapons
				//If Cliked on Lolly Bomb
				if ((scene->event.button.x >= LollyBomb_rect.x) && (scene->event.button.x <= LollyBomb_rect.x + LollyBomb_rect.w)
					&& (scene->event.button.y >= LollyBomb_rect.y) && (scene->event.button.y <= LollyBomb_rect.y + LollyBomb_rect.h) && Choosed_LollyBomb != true)
				{
					Weapon *weapon = NULL;
					weapon = (Weapon *)malloc(sizeof(Weapon));
					strcpy_s(weapon->name, NAME_LENGTH, "Lolly Bomb");
					weapon->score = 1;
					weapon->angle = 0;
					weapon->gravitatin = 0;
					weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
					PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
					Choosed_LollyBomb = true;
					Selected_A_Weapon = true;
					SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
					SDL_RenderFillRect(scene->renderer, &LollyBomb_rect);
					DrawWeaponInsidePanel(scene, "Lolly Bomb", 10);
					if (Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
					{
						WeaponsChoosed = true;
						Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Landscape...", colorBlack, colorBg, "Center", 7);
					}
					SDL_RenderPresent(scene->renderer);
					break;

				}
				//If Cliked on Laser
				if ((scene->event.button.x >= Laser_rect.x) && (scene->event.button.x <= Laser_rect.x + Laser_rect.w)
					&& (scene->event.button.y >= Laser_rect.y) && (scene->event.button.y <= Laser_rect.y + Laser_rect.h) && Choosed_Laser != true)
				{
					Weapon *weapon = NULL;
					weapon = (Weapon *)malloc(sizeof(Weapon));
					strcpy_s(weapon->name, NAME_LENGTH, "Laser");
					weapon->score = 5;
					weapon->angle = 0;
					weapon->gravitatin = 0;
					weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
					PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
					Choosed_Laser = true;
					Selected_A_Weapon = true;
					SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
					SDL_RenderFillRect(scene->renderer, &Laser_rect);
					DrawWeaponInsidePanel(scene, "Laser", 10);
					SDL_RenderPresent(scene->renderer);
					if (Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
					{
						WeaponsChoosed = true;
						Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Landscape...", colorBlack, colorBg, "Center", 7);
					}
					SDL_RenderPresent(scene->renderer);
					break;
				}
				//If Cliked on Ravine
				if ((scene->event.button.x >= Ravine_rect.x) && (scene->event.button.x <= Ravine_rect.x + Ravine_rect.w)
					&& (scene->event.button.y >= Ravine_rect.y) && (scene->event.button.y <= Ravine_rect.y + Ravine_rect.h) && Choosed_Ravine != true)
				{
					Weapon *weapon = NULL;
					weapon = (Weapon *)malloc(sizeof(Weapon));
					strcpy_s(weapon->name, NAME_LENGTH, "Ravine");
					weapon->score = 0;
					weapon->angle = 0;
					weapon->gravitatin = 0;
					weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
					PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
					/*PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);*/
					Choosed_Ravine = true;
					Selected_A_Weapon = true;
					SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
					SDL_RenderFillRect(scene->renderer, &Ravine_rect);
					DrawWeaponInsidePanel(scene, "Ravine", 10);
					SDL_RenderPresent(scene->renderer);
					if (Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
					{
						WeaponsChoosed = true;
						Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Landscape...", colorBlack, colorBg, "Center", 7);
					}
					SDL_RenderPresent(scene->renderer);
					break;
				}
				//If Cliked on Lolly Bomb 2.0
				if ((scene->event.button.x >= LollyBomb2_rect.x) && (scene->event.button.x <= LollyBomb2_rect.x + LollyBomb2_rect.w)
					&& (scene->event.button.y >= LollyBomb2_rect.y) && (scene->event.button.y <= LollyBomb2_rect.y + LollyBomb2_rect.h) && Choosed_LollyBomb2 != true)
				{
					Weapon *weapon = NULL;
					weapon = (Weapon *)malloc(sizeof(Weapon));
					strcpy_s(weapon->name, NAME_LENGTH, "Lolly Bomb 2.0");
					weapon->score = 2;
					weapon->angle = 0;
					weapon->gravitatin = 0;
					weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
					PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
					/*PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);*/
					Choosed_LollyBomb2 = true;
					Selected_A_Weapon = true;
					SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
					SDL_RenderFillRect(scene->renderer, &LollyBomb2_rect);
					DrawWeaponInsidePanel(scene, "Lolly Bomb 2.0", 8);
					SDL_RenderPresent(scene->renderer);
					if (Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
					{
						WeaponsChoosed = true;
						Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Landscape...", colorBlack, colorBg, "Center", 7);
					}
					SDL_RenderPresent(scene->renderer);
					break;
				}
				//If Cliked on Chinese Wall 
				if ((scene->event.button.x >= ChineseWall_rect.x) && (scene->event.button.x <= ChineseWall_rect.x + ChineseWall_rect.w)
					&& (scene->event.button.y >= ChineseWall_rect.y) && (scene->event.button.y <= ChineseWall_rect.y + ChineseWall_rect.h) && Choosed_ChineseWall != true)
				{
					Weapon *weapon = NULL;
					weapon = (Weapon *)malloc(sizeof(Weapon));
					strcpy_s(weapon->name, NAME_LENGTH, "Chinese Wall");
					weapon->score = 0;
					weapon->angle = 0;
					weapon->gravitatin = 0;
					weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
					PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
					/*PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);*/
					Choosed_ChineseWall = true;
					Selected_A_Weapon = true;
					SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
					SDL_RenderFillRect(scene->renderer, &ChineseWall_rect);
					DrawWeaponInsidePanel(scene, "Chinese Wall", 8);
					SDL_RenderPresent(scene->renderer);
					if (Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
					{
						WeaponsChoosed = true;
						Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Landscape...", colorBlack, colorBg, "Center", 7);
					}
					SDL_RenderPresent(scene->renderer);
					break;
				}
				//If Cliked on Pineaple
				if ((scene->event.button.x >= Pineaple_rect.x) && (scene->event.button.x <= Pineaple_rect.x + Pineaple_rect.w)
					&& (scene->event.button.y >= Pineaple_rect.y) && (scene->event.button.y <= Pineaple_rect.y + Pineaple_rect.h) && Choosed_Pineaple != true)
				{
					Weapon *weapon = NULL;
					weapon = (Weapon *)malloc(sizeof(Weapon));
					strcpy_s(weapon->name, NAME_LENGTH, "Pineaple");
					weapon->score = 4;
					weapon->angle = 0;
					weapon->gravitatin = 0;
					weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
					PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
					/*PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);*/
					Choosed_Pineaple = true;
					Selected_A_Weapon = true;
					SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
					SDL_RenderFillRect(scene->renderer, &Pineaple_rect);
					DrawWeaponInsidePanel(scene, "Pineaple", 10);
					SDL_RenderPresent(scene->renderer);
					if (Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
					{
						WeaponsChoosed = true;
						Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Choose Landscape...", colorBlack, colorBg, "Center", 7);
					}
					SDL_RenderPresent(scene->renderer);
					break;
				}
				//If all weapons have been choosed
			}
		}
		if (Selected_A_Weapon == true && scene->playerLap == 1)
		{
			scene->playerLap = 2;
			Selected_A_Weapon = false;
		}
		else if (Selected_A_Weapon == true && scene->playerLap == 2)
		{
			scene->playerLap = 1;
			Selected_A_Weapon = false;
		}
	}

	//Event for choosing Lanscape
	SDL_RenderPresent(scene->renderer);
	bool LandscapeChoosed = false;
	while (LandscapeChoosed != true)
	{
		while (SDL_PollEvent(&scene->event))
		{
			if (scene->event.type == SDL_QUIT)ExitWhileInMenu(scene);
			if (scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT)
			{
				//Event for button "Quit The Game"
				if ((scene->event.button.x >= ButtonQuitTheGame_rect.x) && (scene->event.button.x <= ButtonQuitTheGame_rect.x + ButtonQuitTheGame_rect.w)
					&& (scene->event.button.y >= ButtonQuitTheGame_rect.y) && (scene->event.button.y <= ButtonQuitTheGame_rect.y + ButtonQuitTheGame_rect.h))
					ExitWhileInMenu(scene);
				//Event for the Lanscape
				if ((scene->event.button.x >= Landscape1_rect.x) && (scene->event.button.x <= Landscape1_rect.x + Landscape1_rect.w)
					&& (scene->event.button.y >= Landscape1_rect.y) && (scene->event.button.y <= Landscape1_rect.y + Landscape1_rect.h))
				{
					scene->landscapeType = 1;
					LandscapeChoosed = true;
					break;
				}
				else
				{
					if ((scene->event.button.x >= Landscape2_rect.x) && (scene->event.button.x <= Landscape2_rect.x + Landscape2_rect.w)
						&& (scene->event.button.y >= Landscape2_rect.y) && (scene->event.button.y <= Landscape2_rect.y + Landscape2_rect.h))
					{
						scene->landscapeType = 2;
						LandscapeChoosed = true;
						break;
					}
					if ((scene->event.button.x >= Landscape3_rect.x) && (scene->event.button.x <= Landscape3_rect.x + Landscape3_rect.w)
						&& (scene->event.button.y >= Landscape3_rect.y) && (scene->event.button.y <= Landscape3_rect.y + Landscape3_rect.h))
					{
						scene->landscapeType = 3;
						LandscapeChoosed = true;
						break;
					}
					if ((scene->event.button.x >= Landscape4_rect.x) && (scene->event.button.x <= Landscape4_rect.x + Landscape4_rect.w)
						&& (scene->event.button.y >= Landscape4_rect.y) && (scene->event.button.y <= Landscape4_rect.y + Landscape4_rect.h))
					{
						scene->landscapeType = 4;
						LandscapeChoosed = true;
						break;
					}
				}
			}
		}
	}

	//Event for clicking on button "Play"
	if (WeaponsChoosed == true && LandscapeChoosed == true)
	{
		//Change the color of the button "Play" and check for the click on it.
		Draw_A_text(scene, WordCurrentPlayerChoosing_rect, "Click On Play...", colorBlack, colorBg, "Center", 8);

		SDL_Color colorButtonPlay2 = { 0, 102, 0 };
		Draw_A_text(scene, ButtonPlay_rect, "Play", colorBlack, colorButtonPlay2, "Center", 10);
		SDL_RenderPresent(scene->renderer);

		bool ClickOnPlay = false;
		while (ClickOnPlay != true)
		{
			while (SDL_PollEvent(&scene->event))
			{
				if (scene->event.type == SDL_QUIT)ExitWhileInMenu(scene);
				if (scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT)
				{
					//Click on "Quit The Game"
					if ((scene->event.button.x >= ButtonQuitTheGame_rect.x) && (scene->event.button.x <= ButtonQuitTheGame_rect.x + ButtonQuitTheGame_rect.w)
						&& (scene->event.button.y >= ButtonQuitTheGame_rect.y) && (scene->event.button.y <= ButtonQuitTheGame_rect.y + ButtonQuitTheGame_rect.h))
						ExitWhileInMenu(scene);
					//Click On "Play"
					if ((scene->event.button.x >= ButtonPlay_rect.x) && (scene->event.button.x <= ButtonPlay_rect.x + ButtonPlay_rect.w)
						&& (scene->event.button.y >= ButtonPlay_rect.y) && (scene->event.button.y <= ButtonPlay_rect.y + ButtonPlay_rect.h))
					{
						ClickOnPlay = true;
						scene->quitThirdScreen = true;
						scene->quitSecondScreen = true;
						Mix_FreeMusic(scene->musique);
						scene->FirstRenderOfParty = true;
						break;
					}
				}
			}
		}
	}
}

void DrawWeaponInsidePanel(Scene *scene, const char *NameWeapon, int sizeOfOneChar)
{
	SDL_Color colorBlack = { 0, 0, 0 };
	SDL_Color colorScoresBg = { 128, 128, 0 };
	SDL_Rect Player1_rect = { (int)(SCREEN_WIDTH / 30.0) + 1, (int)((3.0 / 40)*SCREEN_HEIGHT) + 1, (int)(SCREEN_WIDTH / 8.0) - 2, (int)(SCREEN_HEIGHT / 20.0) - 2 };
	SDL_Rect Player2_rect = { (int)((77.0 / 120)*SCREEN_WIDTH) + 1, Player1_rect.y, Player1_rect.w, Player1_rect.h };
	SDL_Rect BlancForWeapons1_rect = { Player1_rect.x, (int)((11.0 / 80)*SCREEN_HEIGHT), Player1_rect.w, (int)((3.0 / 5)*SCREEN_HEIGHT) };
	SDL_Rect BlancForWeapons2_rect = { Player2_rect.x, BlancForWeapons1_rect.y, BlancForWeapons1_rect.w, BlancForWeapons1_rect.h };
	SDL_Rect WeaponOnPlayerList_rect;
	if (scene->playerLap == 1)
	{
		WeaponOnPlayerList_rect = { BlancForWeapons1_rect.x, BlancForWeapons1_rect.y + scene->players[scene->playerLap - 1].NbrWeapons * (int)(BlancForWeapons1_rect.h / 10.0), BlancForWeapons1_rect.w,  (int)(BlancForWeapons1_rect.h / 10.0) };
		Draw_A_text(scene, WeaponOnPlayerList_rect, NameWeapon, colorBlack, colorScoresBg, "Center", sizeOfOneChar);
	}
	if (scene->playerLap == 2)
	{
		WeaponOnPlayerList_rect = { BlancForWeapons2_rect.x, BlancForWeapons2_rect.y + scene->players[scene->playerLap - 1].NbrWeapons * (int)(BlancForWeapons2_rect.h / 10.0), BlancForWeapons2_rect.w,  (int)(BlancForWeapons2_rect.h / 10.0) };
		Draw_A_text(scene, WeaponOnPlayerList_rect, NameWeapon, colorBlack, colorScoresBg, "Center", sizeOfOneChar);
	}
	scene->players[scene->playerLap - 1].NbrWeapons++;
}


int ExitWhileInMenu(Scene *scene)
{
	SDL_DestroyRenderer(scene->renderer);
	SDL_DestroyWindow(scene->window);
	TTF_CloseFont(scene->font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}

SDL_Texture * CreateTextureFromText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color fg, SDL_Color bg)
{
	SDL_Surface * surfaceText = TTF_RenderText_Shaded(font, text, fg, bg);
	SDL_Texture * textureText = SDL_CreateTextureFromSurface(renderer, surfaceText);
	SDL_FreeSurface(surfaceText);
	return textureText;
}

SDL_Texture * CreateTextureFromNumber(SDL_Renderer *renderer, TTF_Font *font, int Number, SDL_Color fg, SDL_Color bg)
{
	char text[20];
	_itoa_s(Number, text, 10);
	SDL_Surface * surfaceNumber = TTF_RenderText_Shaded(font, text, fg, bg);
	SDL_Texture * textureNumber = SDL_CreateTextureFromSurface(renderer, surfaceNumber);
	SDL_FreeSurface(surfaceNumber);
	return textureNumber;
}


void InitTopPanels(Scene *scene)
{
	SDL_Color fg = { 0, 0, 0 };
	SDL_Color bg = { 160, 200, 160 };
	SDL_Color colorGold = { 255, 215, 0 };
	// Panel of Player 1
	scene->topPanels[0].rect = { 0, 0, (int)0.3*SCREEN_WIDTH, (int)0.15*SCREEN_HEIGHT };
	// Panel of Player 2
	scene->topPanels[1].rect = { SCREEN_WIDTH - scene->topPanels[0].rect.w, scene->topPanels[0].rect.y, scene->topPanels[0].rect.w, scene->topPanels[0].rect.h };
	for (int i = 0; i <= 1; i++) {
		//Creating Textures For Names and Scores  
		scene->players[i].textureNamePlayer = CreateTextureFromText(scene->renderer, scene->font, scene->players[i].name, fg, bg);
		scene->players[i].textureScorePlayer = CreateTextureFromNumber(scene->renderer, scene->font, scene->players[i].score, fg, bg);
	}
	scene->texureWordScore = CreateTextureFromText(scene->renderer, scene->font, "Score: ", colorGold, bg);
}

void DrawTopPanels(Scene *scene)
{
	SDL_Color fg = { 0, 0, 0 };
	SDL_Color bg = { 160, 200, 160 };
	SDL_Color colorGold = { 255, 215, 0 };
	int Name_x = 0, Name_y = 0;
	int Text_Width = SCREEN_WIDTH / 8;
	int Text_Height = (2 * SCREEN_HEIGHT) / 15;
	for (int i = 0; i <= 1; i++) {
		if (scene->players[i].scoreTemp != scene->players[i].score) {
			//Creating a new texture for the score of the player
			SDL_DestroyTexture(scene->players[i].textureScorePlayer);
			scene->players[i].textureScorePlayer = NULL;
			scene->players[i].textureScorePlayer = CreateTextureFromNumber(scene->renderer, scene->font, scene->players[i].score, colorGold, bg);
		}
		if (i == 1) Name_x = SCREEN_WIDTH - Text_Width - Name_x;
		SDL_Rect Name_rect = { Name_x, Name_y, Text_Width, Text_Height / 2 };
		SDL_Rect WordScore_rect = { Name_x, Name_y + Name_rect.h, Text_Width / 2, Text_Height / 2 };
		SDL_Rect Score_rect = { Name_x + Text_Width / 2,Name_y + Name_rect.h, Text_Width / 2, Text_Height / 2 };
		SDL_RenderCopy(scene->renderer, scene->players[i].textureNamePlayer, NULL, &Name_rect);
		SDL_RenderCopy(scene->renderer, scene->texureWordScore, NULL, &WordScore_rect);
		SDL_RenderCopy(scene->renderer, scene->players[i].textureScorePlayer, NULL, &Score_rect);
	}
	//SDL_RenderPresent(scene->renderer);
}

void CreateAndDrawBottomPanels(Scene *scene, TTF_Font *font, Player players[])
{
	SDL_Color fg = { 0, 0, 0 };
	SDL_Color bg = { 0,102,0 };
	//Creating Texture For Bottom Panel's Background
	SDL_Texture * textureBottomPanelBackground = LoadTexture(scene->renderer, "Sprites/Green_Background_Camouflage_BottomPanel.BMP");

	SDL_Texture * ButtonUp = LoadTexture(scene->renderer, "Sprites/Green_Arrow_Up.BMP");
	SDL_Texture * ButtonDown = LoadTexture(scene->renderer, "Sprites/Green_Arrow_Down.BMP");
	SDL_Texture * ButtonLeft = LoadTexture(scene->renderer, "Sprites/Green_Arrow_Left.BMP");
	SDL_Texture * ButtonRight = LoadTexture(scene->renderer, "Sprites/Green_Arrow_Right.BMP");
	SDL_Texture * textureEricAndHank = LoadTexture(scene->renderer, "Sprites/EricAndHank.BMP");

	SDL_Texture* textureBottomPanel = SDL_CreateTexture(scene->renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_Rect BottomPanel_rect = { 0, (int)(80 * SCREEN_HEIGHT) / 100, SCREEN_WIDTH, (int)(20 * SCREEN_WIDTH) / 100 };
	SDL_RenderCopy(scene->renderer, textureBottomPanelBackground, NULL, &BottomPanel_rect);

	SDL_Rect EricAndHank_rect = { (int)((40 + 2.5)*SCREEN_WIDTH) / 100, (int)(80 * SCREEN_HEIGHT) / 100, (int)(15 * SCREEN_WIDTH) / 100, (int)(20 * SCREEN_HEIGHT) / 100 };
	SDL_RenderCopy(scene->renderer, textureEricAndHank, NULL, &EricAndHank_rect);

	int Button1_x, Button1_y, Button1_w, Button1_h, Button2_x, Button2_y, Button2_w, Button2_h, Button3_x, Button3_y, Button3_w, Button3_h;
	const char *textName;
	std::string textPuissance;
	for (int i = 0; i <= 1; i++)
	{
		if (i == 0)
		{
			Button1_x = (int)SCREEN_WIDTH / 30;
			Button1_y = (int)SCREEN_HEIGHT*(33.0 / 40);
			Button1_w = Button1_x * 4;
			Button1_h = (int)SCREEN_HEIGHT / 20.0;

			Button2_x = Button1_x * 6;
			Button2_y = Button1_y;
			Button2_w = (int)Button1_w / 2;
			Button2_h = Button1_h;

			Button3_x = (int)SCREEN_WIDTH*(3.0 / 10);
			Button3_y = Button2_y + Button2_h;
			Button3_w = (int)Button2_w / 3;
			Button3_h = Button1_h;
		}
		else {
			Button1_x = (int)SCREEN_WIDTH*(5.0 / 6);
			Button1_y = (int)SCREEN_HEIGHT*(33.0 / 40);
			Button1_w = (int)SCREEN_WIDTH*(2.0 / 15);
			Button1_h = (int)SCREEN_HEIGHT / 20.0;

			Button2_x = (int)SCREEN_WIDTH*(11.0 / 15);
			Button2_y = Button1_y;
			Button2_w = (int)Button1_w / 2;
			Button2_h = Button1_h;

			Button3_x = (int)SCREEN_WIDTH*(19.0 / 30);
			Button3_y = Button2_y + Button2_h;
			Button3_w = (int)Button2_w / 3;
			Button3_h = Button1_h;
		}
		SDL_Rect Button1Up_rect = { Button1_x, Button1_y, Button1_w, Button1_h };
		SDL_Rect Gun_rect = { Button1_x, Button1_y + Button1_h, Button1_w, Button1_h };
		SDL_Rect Button1Down_rect = { Button1_x, Button1_y + Button1_h * 2, Button1_w, Button1_h };

		SDL_Rect Button2Up_rect = { Button2_x, Button2_y, Button2_w, Button2_h };
		SDL_Rect Angle_rect = { Button2_x, Button2_y + Button2_h, Button2_w, Button2_h };
		SDL_Rect Button2Down_rect = { Button2_x, Button2_y + Button2_h * 2, Button2_w, Button2_h };

		SDL_Rect Button3Left_rect = { Button3_x, Button3_y, Button3_w, Button3_h };
		SDL_Rect Power_rect = { Button3_x + Button3_w, Button3_y, Button3_w, Button3_h };
		SDL_Rect Button3Right_rect = { Button3_x + Button3_w * 2, Button3_y, Button3_w, Button3_h };

		// Ïîñëå âûñòðåëà ïîñëåäíåãî îðóæèÿ headWeapon = NULL, ïîýòîìó íå ïîëó÷àåòñÿ ïîëó÷èòü íàçâàíèå îðóæèÿ, èç-çà ýòîãî ïðîèñõîäèò îøèáêà.
		//if (players[i].headWeapon != NULL)  //
		//{
		//	//Creating Texture For Gun's Name
		//	SDL_Texture * textureGunName = CreateTextureFromText(renderer, font, players[i].headWeapon->name, fg, bg);
		//	//Creating Texture For Gun's Angle
		//	SDL_Texture * textureGunAngle = NULL;
		//	if (i == 0) textureGunAngle = CreateTextureFromNumber(renderer, font, (-1)*players[i].tank.cannon.angle, fg, bg);
		//	else textureGunAngle = CreateTextureFromNumber(renderer, font, players[i].tank.cannon.angle, fg, bg);
		//	//Creating Texture For Gun's Puissance
		//	SDL_Texture * textureGunPuissance = CreateTextureFromNumber(renderer, font, players[i].headWeapon->power, fg, bg);
		//	DrawBottomPanel(renderer, textureBottomPanel, ButtonUp, ButtonDown, ButtonLeft, ButtonRight, textureGunName, textureGunAngle, textureGunPuissance, i);
		//	SDL_DestroyTexture(textureGunName);
		//	SDL_DestroyTexture(textureGunAngle);
		//	SDL_DestroyTexture(textureGunPuissance);
		//}
		//else
		//{
		//	//Creating Texture For Gun's Name
		//	SDL_Texture * textureGunName = CreateTextureFromText(renderer, font, "NULL", fg, bg);
		//	//Creating Texture For Gun's Angle
		//	SDL_Texture * textureGunAngle = NULL;
		//	if (i == 0) textureGunAngle = CreateTextureFromNumber(renderer, font, 0, fg, bg);
		//	else textureGunAngle = CreateTextureFromNumber(renderer, font, 0, fg, bg);
		//	//Creating Texture For Gun's Puissance
		//	SDL_Texture * textureGunPuissance = CreateTextureFromNumber(renderer, font, 0, fg, bg);
		//	DrawBottomPanel(renderer, textureBottomPanel, ButtonUp, ButtonDown, ButtonLeft, ButtonRight, textureGunName, textureGunAngle, textureGunPuissance, i);
		//	SDL_DestroyTexture(textureGunName);
		//	SDL_DestroyTexture(textureGunAngle);
		//	SDL_DestroyTexture(textureGunPuissance);
		//}

		if (players[i].headWeapon != NULL)
		{
			//Obtaining Gun's Name and Gun's Puissance
			textName = players[i].headWeapon->name;
			textPuissance = std::to_string(players[i].power);
		}
		else
		{
			//Obtaining Gun's Name and Gun's Puissance
			textName = "Empty";
			textPuissance = "0";
		}
		//Obtaining Gun's Angle
		std::string textAngle;
		if (i == 0) textAngle = std::to_string((-1)*players[i].tank.cannon.angle);
		else textAngle = std::to_string(players[i].tank.cannon.angle - 180);

		SDL_RenderCopy(scene->renderer, ButtonUp, NULL, &Button1Up_rect);
		if (strlen(textName) > 10)Draw_A_text(scene, Gun_rect, textName, fg, bg, "Center", 9);
		else Draw_A_text(scene, Gun_rect, textName, fg, bg, "Center", 12);
		SDL_RenderCopy(scene->renderer, ButtonDown, NULL, &Button1Down_rect);

		SDL_RenderCopy(scene->renderer, ButtonUp, NULL, &Button2Up_rect);
		Draw_A_text(scene, Angle_rect, textAngle.c_str(), fg, bg, "Center", 20);
		SDL_RenderCopy(scene->renderer, ButtonDown, NULL, &Button2Down_rect);

		SDL_RenderCopy(scene->renderer, ButtonLeft, NULL, &Button3Left_rect);
		if (textPuissance.length() <= 2) Draw_A_text(scene, Power_rect, textPuissance.c_str(), fg, bg, "Center", 12);
		else  Draw_A_text(scene, Power_rect, textPuissance.c_str(), fg, bg, "Center", 8);
		SDL_RenderCopy(scene->renderer, ButtonRight, NULL, &Button3Right_rect);
	}
	SDL_SetRenderTarget(scene->renderer, textureBottomPanel);
	SDL_DestroyTexture(ButtonUp);
	SDL_DestroyTexture(ButtonDown);
	SDL_DestroyTexture(ButtonLeft);
	SDL_DestroyTexture(ButtonRight);
	SDL_DestroyTexture(textureEricAndHank);
	SDL_DestroyTexture(textureBottomPanelBackground);
	SDL_SetRenderTarget(scene->renderer, NULL);
	SDL_DestroyTexture(textureBottomPanel);
}

void BottomPanelInterations(Player players[], int Mouse_x, int Mouse_y, int PlayerLap, Weapon *activeWeapon)
{
	int Button1_x, Button1_y, Button1_w, Button1_h, Button2_x, Button2_y, Button2_w, Button2_h, Button3_x, Button3_y, Button3_w, Button3_h;
	if (PlayerLap == 1)
	{
		Button1_x = (int)SCREEN_WIDTH / 30;
		Button1_y = (int)SCREEN_HEIGHT*(33.0 / 40);
		Button1_w = Button1_x * 4;
		Button1_h = (int)SCREEN_HEIGHT / 20.0;

		Button2_x = Button1_x * 6;
		Button2_y = Button1_y;
		Button2_w = (int)Button1_w / 2;
		Button2_h = Button1_h;

		Button3_x = (int)SCREEN_WIDTH*(3.0 / 10);
		Button3_y = Button2_y + Button2_h;
		Button3_w = (int)Button2_w / 3;
		Button3_h = Button1_h;
	}
	else {
		Button1_x = (int)SCREEN_WIDTH*(5.0 / 6);
		Button1_y = (int)SCREEN_HEIGHT*(33.0 / 40);
		Button1_w = (int)SCREEN_WIDTH*(2.0 / 15);
		Button1_h = (int)SCREEN_HEIGHT / 20.0;

		Button2_x = (int)SCREEN_WIDTH*(11.0 / 15);
		Button2_y = Button1_y;
		Button2_w = (int)Button1_w / 2;
		Button2_h = Button1_h;

		Button3_x = (int)SCREEN_WIDTH*(19.0 / 30);
		Button3_y = Button2_y + Button2_h;
		Button3_w = (int)Button2_w / 3;
		Button3_h = Button1_h;
	}
	SDL_Rect Button1Up_rect = { Button1_x, Button1_y, Button1_w, Button1_h };
	SDL_Rect Button1Down_rect = { Button1_x, Button1_y + Button1_h * 2, Button1_w, Button1_h };

	SDL_Rect Button2Up_rect = { Button2_x, Button2_y, Button2_w, Button2_h };
	SDL_Rect Button2Down_rect = { Button2_x, Button2_y + Button2_h * 2, Button2_w, Button2_h };

	SDL_Rect Button3Left_rect = { Button3_x, Button3_y, Button3_w, Button3_h };
	SDL_Rect Button3Right_rect = { Button3_x + Button3_w * 2, Button3_y, Button3_w, Button3_h };

	if ((Mouse_x >= Button1Up_rect.x && Mouse_x <= Button1Up_rect.x + Button1Up_rect.w) && (Mouse_y >= Button1Up_rect.y && Mouse_y <= Button1Up_rect.y + Button1Up_rect.h))
	{
		//Load The Next Gun
		SetHeadOnNext(&players[PlayerLap - 1].headWeapon);
	}
	if ((Mouse_x >= Button1Down_rect.x && Mouse_x <= Button1Down_rect.x + Button1Down_rect.w) && (Mouse_y >= Button1Down_rect.y && Mouse_y <= Button1Down_rect.y + Button1Down_rect.h))
	{
		//Load The Previous Gun
		SetHeadOnPrev(&players[PlayerLap - 1].headWeapon);
	}
	if ((Mouse_x >= Button2Up_rect.x && Mouse_x <= Button2Up_rect.x + Button2Up_rect.w) && (Mouse_y >= Button2Up_rect.y && Mouse_y <= Button2Up_rect.y + Button2Up_rect.h))
	{
		if (PlayerLap == 1)
		{
			if ((players[PlayerLap - 1].tank.cannon.angle * (-1)) < 90) players[PlayerLap - 1].tank.cannon.angle--;
		}
		if (PlayerLap == 2)
		{
			if (players[PlayerLap - 1].tank.cannon.angle < 90) players[PlayerLap - 1].tank.cannon.angle++;
		}
	}
	if ((Mouse_x >= Button2Down_rect.x && Mouse_x <= Button2Down_rect.x + Button2Down_rect.w) && (Mouse_y >= Button2Down_rect.y && Mouse_y <= Button2Down_rect.y + Button2Down_rect.h))
	{
		if (PlayerLap == 1)
		{
			if (players[PlayerLap - 1].tank.cannon.angle < 0) players[PlayerLap - 1].tank.cannon.angle++;
		}
		if (PlayerLap == 2)
		{
			if (players[PlayerLap - 1].tank.cannon.angle > 0) players[PlayerLap - 1].tank.cannon.angle++;
		}
	}
	if ((Mouse_x >= Button3Left_rect.x && Mouse_x <= Button3Left_rect.x + Button3Left_rect.w) && (Mouse_y >= Button3Left_rect.y && Mouse_y <= Button3Left_rect.y + Button3Left_rect.h))
	{
		if (players[PlayerLap - 1].power > 0) players[PlayerLap - 1].power--; //
	}
	if ((Mouse_x >= Button3Right_rect.x && Mouse_x <= Button3Right_rect.x + Button3Right_rect.w) && (Mouse_y >= Button3Right_rect.y && Mouse_y <= Button3Right_rect.y + Button3Right_rect.h))
	{
		if (players[PlayerLap - 1].power < 100) players[PlayerLap - 1].power++; //
	}
}


void ObtainNameOfWinner(Scene *scene, int i)
{
	SDL_Color colorGameBg = { 160, 200, 160, 0 };
	SDL_Color colorBlack = { 0, 0, 0, };
	SDL_Color colorGold = { 255, 215, 0 };
	SDL_Rect Congratulation_rect = { (int)((SCREEN_WIDTH / 2.0) - ((43 * 12) / 2.0)), (int)((17.0 / 60)*SCREEN_HEIGHT * (1.0 / 2)), 3 * (int)(SCREEN_WIDTH / 6.0), (int)(SCREEN_HEIGHT / 12.0) };
	SDL_Rect BlancForName_rect = { (int)((SCREEN_WIDTH / 2.0) - ((SCREEN_WIDTH / 6.0) / 2.0)), Congratulation_rect.y + Congratulation_rect.h + 30, (int)(SCREEN_WIDTH / 6.0), Congratulation_rect.h };
	std::string s = ("Player ");
	s.append(std::to_string(i + 1));
	s.append(" have Won! ");
	s.append(" Enter the winner's name: ");
	Draw_A_text(scene, Congratulation_rect, s.c_str(), colorBlack, colorGameBg, "Center", 12);
	SDL_RenderPresent(scene->renderer);

	s = "";
	bool keyIsAlreadyPressed = false;
	bool quit = false;
	while (quit == false) {
		while (SDL_PollEvent(&scene->event)) {
			if (scene->event.key.keysym.sym == SDLK_RETURN || scene->event.type == SDL_QUIT) {
				quit = true;
				break;
			}
			if (scene->event.type == SDL_KEYDOWN) {
				if (s.length() >= 6) {
					continue;
				}
				if (keyIsAlreadyPressed == false && scene->event.key.keysym.scancode >= SDL_SCANCODE_A && scene->event.key.keysym.scancode <= SDL_SCANCODE_0)
				{
					const char *c = (SDL_GetKeyName(scene->event.key.keysym.sym));
					s.append(c);
					Draw_A_text(scene, BlancForName_rect, s.c_str(), colorGold, colorGameBg, "Center", 18);
					SDL_RenderPresent(scene->renderer);
					keyIsAlreadyPressed = false;
				}
			}
			else if (scene->event.type == SDL_KEYUP) {
				keyIsAlreadyPressed = false;
			}
		}
	}
	strcpy_s(scene->WinnerName, s.c_str());
}

void UpdateAndSaveRecord(Scene *scene, Player players[], int i)
{
	RecordRow records[NUMBER_OF_RECORD_ROWS];
	LoadRecords(records);
	int recordsIndex = -1;
	for (int j = 0; j < NUMBER_OF_RECORD_ROWS; j++)
		if (players[i].score >= records[j].score)
		{
			recordsIndex = j;
			break;
		}
	if (recordsIndex > -1)
	{
		for (int j = NUMBER_OF_RECORD_ROWS - 2; j >= recordsIndex; j--) records[j + 1] = records[j];
		strcpy_s(records[recordsIndex].name, NAME_LENGTH, scene->WinnerName);
		records[recordsIndex].score = players[i].score;
	}
	FILE *recordsFile = NULL;
	fopen_s(&recordsFile, "records.pt", "wb");
	fwrite(records, sizeof(RecordRow), NUMBER_OF_RECORD_ROWS, recordsFile);
	fclose(recordsFile);
	recordsFile = NULL;
}