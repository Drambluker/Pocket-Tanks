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

	int landscapeType;

	printf_s("Landscape type: ");
	scanf_s("%d", &landscapeType);

	(*scene).window = SDL_CreateWindow("Pocket Tanks", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (!(*scene).window)
	{
		printf_s("SDL_Error: %s\n", SDL_GetError());
		system("pause");
		exit(1);
	}

	(*scene).renderer = SDL_CreateRenderer((*scene).window, -1, 0);

	if (!(*scene).renderer)
	{
		printf_s("SDL_Error: %s\n", SDL_GetError());
		system("pause");
		exit(1);
	}

	(*scene).font = TTF_OpenFont("arial.ttf", 200);

	if (!(*scene).font)
	{
		printf_s("TTF_Error: %s\n", TTF_GetError());
		system("pause");
		exit(1);
	}

	InitLandscape(&scene->landscape, &scene->defaultLandscape, landscapeType);
	InitPlayers((*scene).players);
	InitTopPanels((*scene).topPanels);

	LoadTextures((*scene).renderer, (*scene).players);

	scene->playerLap = 1;
	scene->activeWeapon = NULL;
	scene->oldTime = scene->newTime = scene->deltaTime = 0;
	scene->timeStart = StartCounter(&scene->PCFreq);
}

bool ProcessEvents(Scene *scene)
{
	(*scene).newTime = GetCounter((*scene).timeStart, (*scene).PCFreq);
	(*scene).deltaTime = (*scene).newTime - (*scene).oldTime;
	(*scene).oldTime = (*scene).newTime;

	while (SDL_PollEvent(&scene->event)) {
		if ((*scene).event.type == SDL_QUIT)
			return true;

		if ((*scene).event.type == SDL_MOUSEBUTTONDOWN && (*scene).event.button.button == SDL_BUTTON_LEFT)
		{
			BottomPanelInterations((*scene).players, (*scene).event.button.x, (*scene).event.button.y, (*scene).playerLap, (*scene).activeWeapon);
		}

		if ((*scene).event.type == SDL_KEYDOWN && (*scene).event.key.keysym.sym == SDLK_UP)
		{
			if ((*scene).playerLap == 1) (*scene).players[(*scene).playerLap - 1].tank.cannon.angle--;
			if ((*scene).playerLap == 2) (*scene).players[(*scene).playerLap - 1].tank.cannon.angle++;
		}

		if ((*scene).event.type == SDL_KEYDOWN && (*scene).event.key.keysym.sym == SDLK_DOWN)
		{
			if ((*scene).playerLap == 1) (*scene).players[(*scene).playerLap - 1].tank.cannon.angle++;
			if ((*scene).playerLap == 2) (*scene).players[(*scene).playerLap - 1].tank.cannon.angle--;
		}

		// if sizechanged - mustRedraw = true;

		if ((*scene).event.type == SDL_KEYDOWN && (*scene).event.key.keysym.sym == SDLK_LEFT && (*scene).players[(*scene).playerLap - 1].tank.body.rect.x > 0)
		{
			(*scene).players[(*scene).playerLap - 1].tank.body.rect.x -= 0.03 * (*scene).deltaTime;
		}

		if ((*scene).event.type == SDL_KEYDOWN && (*scene).event.key.keysym.sym == SDLK_RIGHT && (*scene).players[(*scene).playerLap - 1].tank.body.rect.x + (*scene).players[(*scene).playerLap - 1].tank.body.rect.w * cos((*scene).players[(*scene).playerLap - 1].tank.angle * Pi / 180) < SCREEN_WIDTH)
		{
			(*scene).players[(*scene).playerLap - 1].tank.body.rect.x += 0.06 * (*scene).deltaTime;
		}

		if ((*scene).event.type == SDL_KEYDOWN && (*scene).event.key.keysym.sym == SDLK_SPACE && (*scene).activeWeapon == NULL)
		{
			(*scene).activeWeapon = PopWeapon(&scene->players[(*scene).playerLap - 1].headWeapon);

			if ((*scene).activeWeapon == NULL && (*scene).playerLap == 2)
			{
				return true;
			}

			if ((*scene).playerLap == 1) (*scene).playerLap = 2;
			else (*scene).playerLap = 1;
		}
	}

	return false;
}

void UpdateLogic(Scene *scene)
{
	SDL_SetRenderDrawColor((*scene).renderer, 160, 200, 160, 0);
	SDL_RenderClear((*scene).renderer);

	if ((*scene).players[0].tank.body.rect.x + (*scene).players[0].tank.body.rect.w < SCREEN_WIDTH)
	{
		(*scene).players[0].tank.angle = 180 / Pi * atan((double)((*scene).landscape.points[(*scene).players[0].tank.body.rect.x + (*scene).players[0].tank.body.rect.w].y -
			(*scene).landscape.points[(*scene).players[0].tank.body.rect.x].y) / ((*scene).landscape.points[(*scene).players[0].tank.body.rect.x + (*scene).players[0].tank.body.rect.w].x -
			(*scene).landscape.points[(*scene).players[0].tank.body.rect.x].x));
	}

	if ((*scene).players[1].tank.body.rect.x + (*scene).players[1].tank.body.rect.w < SCREEN_WIDTH)
	{
		(*scene).players[1].tank.angle = 180 / Pi * atan((double)((*scene).landscape.points[(*scene).players[1].tank.body.rect.x + (*scene).players[1].tank.body.rect.w].y -
			(*scene).landscape.points[(*scene).players[1].tank.body.rect.x].y) / ((*scene).landscape.points[(*scene).players[1].tank.body.rect.x + (*scene).players[1].tank.body.rect.w].x -
			(*scene).landscape.points[(*scene).players[1].tank.body.rect.x].x));
	}

	Gravitate((*scene).players, (*scene).landscape);

	if ((*scene).activeWeapon != NULL && GotInTheTank((*scene).activeWeapon, (*scene).players[((*scene).playerLap == 2) ? 1 : 0]))
	{
		(*scene).players[((*scene).playerLap == 2) ? 0 : 1].score += (*scene).activeWeapon->score;
	}

	if ((*scene).activeWeapon != NULL && ((*scene).activeWeapon->rect.y >= (*scene).landscape.points[(*scene).activeWeapon->rect.x].y ||
		GotInTheTank((*scene).activeWeapon, (*scene).players[((*scene).playerLap == 2) ? 1 : 0]) ||
		scene->activeWeapon->rect.x <= 0 ||
		scene->activeWeapon->rect.x >= SCREEN_WIDTH ||
		(scene->activeWeapon->rect.y <= 0 && strcmp(scene->activeWeapon->name, "Laser") == 0) ||
		scene->activeWeapon->rect.y >= SCREEN_HEIGHT)) //
	{
		if (!(GotInTheTank((*scene).activeWeapon, (*scene).players[((*scene).playerLap == 2) ? 1 : 0]) ||
			scene->activeWeapon->rect.x <= 0 ||
			scene->activeWeapon->rect.x >= SCREEN_WIDTH ||
			(scene->activeWeapon->rect.y <= 0 && strcmp(scene->activeWeapon->name, "Laser") == 0) ||
			scene->activeWeapon->rect.y >= SCREEN_HEIGHT))
		{
			SDL_Point depth—oordinate = { scene->activeWeapon->rect.x, scene->activeWeapon->rect.y }; //

			if (strcmp(scene->activeWeapon->name, "Chinese Wall") == 0)
				for (int i = depth—oordinate.x - 5; i < depth—oordinate.x + 5; i++)
					scene->landscape.points[i].y -= 300;
			else if (strcmp(scene->activeWeapon->name, "Ravine") == 0)
			{
				for (int i = depth—oordinate.x - 50; i < depth—oordinate.x + 50; i++)
					scene->landscape.points[i].y += 0.8 * SCREEN_HEIGHT - scene->landscape.points[i].y;
			}
			else if (strcmp(scene->activeWeapon->name, "Laser") != 0)
			{
				double t = 0;

				for (int i = depth—oordinate.x - scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE; i <= depth—oordinate.x + scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE; i++)
				{
					if (scene->landscape.points[depth—oordinate.x].y >= scene->defaultLandscape.points[depth—oordinate.x].y || scene->landscape.points[i].y <= depth—oordinate.y)
						scene->landscape.points[i].y += scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE * sin(t);

					t += Pi / (2 * scene->activeWeapon->score * SIZE_OF_LANDSCAPE_DAMAGE + 1);
				}
			}
		}

		SDL_DestroyTexture((*scene).activeWeapon->texture);
		(*scene).activeWeapon->texture = NULL;
		free((*scene).activeWeapon);
		(*scene).activeWeapon = NULL;
	}
}

void DoRender(Scene *scene)
{
	RenderWeapon((*scene).renderer, (*scene).activeWeapon);
	DrawLandscape((*scene).renderer, (*scene).landscape);
	DrawTanks((*scene).renderer, (*scene).players);

	if ((*scene).activeWeapon != NULL && (*scene).activeWeapon->rect.y < (*scene).landscape.points[(*scene).activeWeapon->rect.x].y)
	{
		(*scene).activeWeapon->rect.x += 0.005 * (*scene).players[((*scene).playerLap == 2) ? 0 : 1].power * cos((*scene).activeWeapon->angle) * (*scene).deltaTime;

		if (strcmp(scene->activeWeapon->name, "Laser") == 0)
		{
			(*scene).activeWeapon->rect.y += 0.005 * (*scene).players[((*scene).playerLap == 2) ? 0 : 1].power * sin((*scene).activeWeapon->angle) * (*scene).deltaTime;
		}
		else
		{
			(*scene).activeWeapon->rect.y += 0.005 * (*scene).players[((*scene).playerLap == 2) ? 0 : 1].power * sin((*scene).activeWeapon->angle) * (*scene).deltaTime + (*scene).activeWeapon->gravitatin * (*scene).deltaTime;
			(*scene).activeWeapon->gravitatin += 0.0025;
		}
	}

	CreateAndDrawTopPanels((*scene).renderer, (*scene).font, (*scene).players, (*scene).topPanels);
	CreateAndDrawBottomPanels((*scene).renderer, (*scene).font, (*scene).players);

	//CreateAndDrawTopPanels(renderer, font, players, topPanels, mustRedraw); //
	//CreateAndDrawBottomPanels(renderer, font, players, mustRedraw); //
	//mustRedraw = false; //

	SDL_RenderPresent((*scene).renderer);
}

void DestroyScene(Scene *scene)
{
	UpdateRecords(scene->players);

	Weapon *tempWeapon = NULL;

	for (int i = 0; i < 2; i++)
	{
		tempWeapon = PopWeapon(&scene->players[i].headWeapon);

		while (tempWeapon != NULL)
		{
			free(tempWeapon);
			tempWeapon = NULL;
			tempWeapon = PopWeapon(&scene->players[i].headWeapon);
		}
	}

	DestroyTextures((*scene).players, (*scene).activeWeapon);

	SDL_DestroyRenderer((*scene).renderer);
	SDL_DestroyWindow((*scene).window);
	TTF_CloseFont((*scene).font);
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

void UpdateRecords(Player players[])
{
	RecordRow records[NUMBER_OF_RECORD_ROWS];
	LoadRecords(records);
	int recordsIndex;

	for (int i = 0; i < 2; i++)
	{
		recordsIndex = -1;

		for (int j = 0; j < NUMBER_OF_RECORD_ROWS; j++)
			if (players[i].score >= records[j].score)
			{
				recordsIndex = j;
				break;
			}

		if (recordsIndex > -1)
		{
			for (int j = NUMBER_OF_RECORD_ROWS - 2; j >= recordsIndex; j--)
				records[j + 1] = records[j];

			strcpy_s(records[recordsIndex].name, NAME_LENGTH, players[i].name);
			records[recordsIndex].score = players[i].score;
		}
	}

	//for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
	//	if (players[0].score >= records[i].score)
	//	{
	//		recordsIndex = i;
	//		break;
	//	}

	//if (recordsIndex > -1)
	//{
	//	for (int i = recordsIndex; i < NUMBER_OF_RECORD_ROWS - 1; i++)
	//		records[i + 1] = records[i];

	//	records[recordsIndex].name = players[0].name;
	//	records[recordsIndex].score = players[0].score;
	//}

	//recordsIndex = -1;

	//for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
	//	if (players[1].score >= records[i].score)
	//	{
	//		recordsIndex = i;
	//		break;
	//	}

	//if (recordsIndex > -1)
	//{
	//	for (int i = recordsIndex; i < NUMBER_OF_RECORD_ROWS - 1; i++)
	//		records[i + 1] = records[i];

	//	records[recordsIndex].name = players[1].name;
	//	records[recordsIndex].score = players[1].score;
	//}

	FILE *recordsFile = NULL;
	fopen_s(&recordsFile, "records.pt", "wb");
	fwrite(records, sizeof(RecordRow), NUMBER_OF_RECORD_ROWS, recordsFile);
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

bool GotInTheTank(Weapon *activeWeapon, Player player)
{
	if (activeWeapon->rect.x > player.tank.body.rect.x && activeWeapon->rect.x < player.tank.body.rect.x + player.tank.body.rect.w &&
		activeWeapon->rect.y > player.tank.body.rect.y && activeWeapon->rect.y < player.tank.body.rect.y + player.tank.body.rect.h)
		return true;

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
		players[i].tank.cannon.angle = 0;
		players[i].tank.angle = 0;
		players[i].headWeapon = NULL;
		players[i].tailWeapon = NULL;

		for (int j = 1; j <= NUMBER_OF_WEAPON; j++)
		{
			weapon = (Weapon *)malloc(sizeof(Weapon));

			switch (j)
			{
			case 1:
				strcpy_s(weapon->name, NAME_LENGTH, "Lolly Bomb");
				weapon->score = 1;
				break;
			case 2:
				strcpy_s(weapon->name, NAME_LENGTH, "Lolly Bomb 2.0");
				weapon->score = 2;
				break;
			case 3:
				strcpy_s(weapon->name, NAME_LENGTH, "Chinese Wall");
				weapon->score = 0;
				break;
			case 4:
				strcpy_s(weapon->name, NAME_LENGTH, "Ravine");
				weapon->score = 0;
				break;
			case 5:
				strcpy_s(weapon->name, NAME_LENGTH, "Laser");
				weapon->score = 5;
				break;
			default:
				break;
			}

			weapon->angle = 0;
			weapon->gravitatin = 0;
			PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		}

		//weapon = (Weapon *)malloc(sizeof(Weapon));
		//weapon->name = "Lolly Bomb";
		//weapon->score = 1;
		//weapon->angle = 0;
		//weapon->gravitatin = 0;
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		//weapon = (Weapon *)malloc(sizeof(Weapon));
		//weapon->name = "Lolly Bomb 2.0";
		//weapon->score = 2;
		//weapon->angle = 0;
		//weapon->gravitatin = 0;
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		//weapon = (Weapon *)malloc(sizeof(Weapon));
		//weapon->name = "Chinese Wall";
		//weapon->score = 0;
		//weapon->angle = 0;
		//weapon->gravitatin = 0;
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		//weapon = (Weapon *)malloc(sizeof(Weapon));
		//weapon->name = "Ravine";
		//weapon->score = 0;
		//weapon->angle = 0;
		//weapon->gravitatin = 0;
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		//weapon->name = "Laser";
		//weapon->score = 5;
		//weapon->angle = 0;
		//weapon->gravitatin = 0;
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
	}

	// Player 1
	strcpy_s(players[0].name, NAME_LENGTH, "Player 1");
	players[0].tank.body.rect = { 10, 125, 75, 45 };

	// Player 2
	strcpy_s(players[1].name, NAME_LENGTH, "Player 2");
	players[1].tank.body.rect = { SCREEN_WIDTH - players[0].tank.body.rect.w - 10, players[0].tank.body.rect.y,
		players[0].tank.body.rect.w, players[0].tank.body.rect.h };
}

void DrawTanks(SDL_Renderer *renderer, Player players[])
{
	// Player 1
	double r = sqrt((players[0].tank.body.rect.x - (players[0].tank.body.rect.x + players[0].tank.body.rect.w / 2)) * (players[0].tank.body.rect.x - (players[0].tank.body.rect.x + players[0].tank.body.rect.w / 2)) +
		(players[0].tank.body.rect.y + players[0].tank.body.rect.h - (players[0].tank.body.rect.y + players[0].tank.body.rect.h / 2)) * (players[0].tank.body.rect.y + players[0].tank.body.rect.h - (players[0].tank.body.rect.y + players[0].tank.body.rect.h / 2)));

	players[0].tank.cannon.rect = { players[0].tank.body.rect.x + (int)(r * cos(2 * Pi - players[0].tank.angle * Pi / 180)),
		players[0].tank.body.rect.y + 130 * players[0].tank.body.rect.h / BODY_H - (int)(r * sin(2 * Pi - players[0].tank.angle * Pi / 180)),
		CANNON_W * players[0].tank.body.rect.w / BODY_W,
		CANNON_H * players[0].tank.body.rect.h / BODY_H };

	SDL_Point pointOfRotation = { 0, players[0].tank.cannon.rect.h / 2 };
	SDL_RenderCopyEx(renderer, players[0].tank.cannon.texture, NULL, &players[0].tank.cannon.rect,
		players[0].tank.cannon.angle, &pointOfRotation, SDL_FLIP_NONE);

	// (375; 130)
	//players[0].tank.cannon.rect = { players[0].tank.body.rect.x + 325 * players[0].tank.body.rect.w / BODY_W,
	//	players[0].tank.body.rect.y + 130 * players[0].tank.body.rect.h / BODY_H,
	//	CANNON_W * players[0].tank.body.rect.w / BODY_W,
	//	CANNON_H * players[0].tank.body.rect.h / BODY_H };
	//pointOfRotation = { -5 - (BODY_W - CANNON_W) * players[0].tank.body.rect.w / BODY_W,
	//5 + (BODY_H - 130) * players[0].tank.body.rect.h / BODY_H };

	if (players[0].headWeapon != NULL)
	{
		players[0].headWeapon->rect = { -2 + players[0].tank.cannon.rect.x + (int)(players[0].tank.cannon.rect.w * cos(2 * Pi - players[0].tank.cannon.angle * Pi / 180)),
			-2 + players[0].tank.cannon.rect.y - (int)(players[0].tank.cannon.rect.w * sin(2 * Pi - players[0].tank.cannon.angle * Pi / 180)),
			7, 7 };
		players[0].headWeapon->angle = players[0].tank.cannon.angle * Pi / 180;
	}

	pointOfRotation = { 0, players[0].tank.body.rect.h };
	SDL_RenderCopyEx(renderer, players[0].tank.body.texture, NULL, &players[0].tank.body.rect,
		players[0].tank.angle, &pointOfRotation, SDL_FLIP_NONE);

	// Player 2
	r = sqrt((players[1].tank.body.rect.x - (players[1].tank.body.rect.x + players[1].tank.body.rect.w / 2)) * (players[1].tank.body.rect.x - (players[1].tank.body.rect.x + players[1].tank.body.rect.w / 2)) +
		(players[1].tank.body.rect.y + players[1].tank.body.rect.h - (players[1].tank.body.rect.y + players[1].tank.body.rect.h / 2)) * (players[1].tank.body.rect.y + players[1].tank.body.rect.h - (players[1].tank.body.rect.y + players[1].tank.body.rect.h / 2)));

	players[1].tank.cannon.rect = { players[1].tank.body.rect.x + (BODY_W - CANNON_W) * players[1].tank.body.rect.w / BODY_W - (int)(r * cos(2 * Pi - players[1].tank.angle * Pi / 180)),
		players[1].tank.body.rect.y + 130 * players[0].tank.body.rect.h / BODY_H - (int)(r * sin(2 * Pi - players[1].tank.angle * Pi / 180)),
		CANNON_W * players[1].tank.body.rect.w / BODY_W,
		CANNON_H * players[1].tank.body.rect.h / BODY_H };

	pointOfRotation = { players[1].tank.cannon.rect.w, players[1].tank.cannon.rect.h / 2 };
	SDL_RenderCopyEx(renderer, players[1].tank.cannon.texture, NULL, &players[1].tank.cannon.rect,
		players[1].tank.cannon.angle, &pointOfRotation, SDL_FLIP_NONE);

	//players[1].tank.cannon.rect = { players[1].tank.body.rect.x + (BODY_W - 325 - CANNON_W) * players[0].tank.body.rect.w / BODY_W,
	//	players[1].tank.body.rect.y + 130 * players[0].tank.body.rect.h / BODY_H,
	//	players[0].tank.cannon.rect.w, players[0].tank.cannon.rect.h };
	//pointOfRotation = { -5 - (BODY_W - 375 - CANNON_W) * players[1].tank.body.rect.w / BODY_W,
	//	5 + (BODY_H - 130) * players[1].tank.body.rect.h / BODY_H };

	if (players[1].headWeapon != NULL)
	{
		players[1].headWeapon->rect = { -2 + players[1].tank.cannon.rect.x + players[1].tank.cannon.rect.w - (int)(players[1].tank.cannon.rect.w * cos(2 * Pi - players[1].tank.cannon.angle * Pi / 180)),
			-2 + players[1].tank.cannon.rect.y - (int)(players[1].tank.cannon.rect.w * sin(players[1].tank.cannon.angle * Pi / 180)),
			7, 7 };
		players[1].headWeapon->angle = Pi + players[1].tank.cannon.angle * Pi / 180;
	}

	pointOfRotation = { 0, players[1].tank.body.rect.h };
	SDL_RenderCopyEx(renderer, players[1].tank.body.texture, NULL, &players[1].tank.body.rect,
		players[1].tank.angle, &pointOfRotation, SDL_FLIP_NONE);
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
	players[1].tank.cannon.texture = LoadTexture(renderer, "Sprites/cannon2.bmp");

	Weapon *weapon = NULL;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < NUMBER_OF_WEAPON; j++)
		{
			weapon = PopWeapon(&players[i].headWeapon);
			weapon->texture = LoadTexture(renderer, "Sprites/weapon1.bmp");
			PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
		}

		//// Lolly Bomb
		//weapon = PopWeapon(&players[i].headWeapon);
		//weapon->texture = LoadTexture(renderer, "Sprites/weapon1.bmp");
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);

		//// Lolly Bomb 2.0
		//weapon = PopWeapon(&players[i].headWeapon);
		//weapon->texture = LoadTexture(renderer, "Sprites/weapon1.bmp");
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);

		//// Chinese Wall
		//weapon = PopWeapon(&players[i].headWeapon);
		//weapon->texture = LoadTexture(renderer, "Sprites/weapon1.bmp");
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);

		//// Ravine
		//weapon = PopWeapon(&players[i].headWeapon);
		//weapon->texture = LoadTexture(renderer, "Sprites/weapon1.bmp");
		//PushWeapon(weapon, &players[i].headWeapon, &players[i].tailWeapon);
	}
}

void DestroyTextures(Player players[], Weapon *activeWeapon)
{
	if (activeWeapon != NULL)
	{
		SDL_DestroyTexture(activeWeapon->texture);
		activeWeapon->texture = NULL;
		free(activeWeapon);
		activeWeapon = NULL;
	}

	Weapon *weapon = NULL;

	for (int i = 1; i >= 0; i--)
	{
		weapon = PopWeapon(&players[i].headWeapon);

		while (weapon != NULL)
		{
			SDL_DestroyTexture(weapon->texture);
			weapon->texture = NULL;
			free(weapon);
			weapon = NULL;
			weapon = PopWeapon(&players[i].headWeapon);
		}

		SDL_DestroyTexture(players[i].tank.cannon.texture);
		players[i].tank.cannon.texture = NULL;
		SDL_DestroyTexture(players[i].tank.body.texture);
		players[i].tank.body.texture = NULL;
	}

	//SDL_DestroyTexture(players[1].tank.cannon.texture);
	//players[1].tank.cannon.texture = NULL;
	//SDL_DestroyTexture(players[1].tank.body.texture);
	//players[1].tank.body.texture = NULL;
	//SDL_DestroyTexture(players[0].tank.cannon.texture);
	//players[0].tank.cannon.texture = NULL;
	//SDL_DestroyTexture(players[0].tank.body.texture);
	//players[0].tank.body.texture = NULL;
}

void RenderWeapon(SDL_Renderer *renderer, Weapon *activeWeapon)
{
	if (activeWeapon != NULL)
	{
		if (strcmp(activeWeapon->name, "Laser") == 0)
		{
			SDL_Point pointOfRotation = { 0, 0 };
			activeWeapon->rect.h = 7;
			activeWeapon->rect.w = 7 * activeWeapon->rect.h;
			SDL_RenderCopyEx(renderer, activeWeapon->texture, NULL, &activeWeapon->rect, activeWeapon->angle * 180 / Pi, &pointOfRotation, SDL_FLIP_NONE);
			return;
		}
		else
			activeWeapon->rect.w = activeWeapon->rect.h = 7;

		SDL_RenderCopy(renderer, activeWeapon->texture, NULL, &activeWeapon->rect);

		//double deltaTime = GetCounter(timeStart, PCFreq) - *oldTime;
		//*oldTime = newTime;
		//activeWeapon->rect.x += 0.005 * activeWeapon->power * cos(activeWeapon->angle) * deltaTime;
		//activeWeapon->rect.y += 0.005 * activeWeapon->power * sin(activeWeapon->angle) * deltaTime + activeWeapon->gravitatin * deltaTime;
		//activeWeapon->gravitatin += 0.01;

		//if (GetTickCount() - *time > 10)
		//{
		//	*time = GetTickCount();
		//	activeWeapon->rect.x += 0.1 * activeWeapon->power * cos(activeWeapon->angle);
		//	activeWeapon->rect.y += 0.1 * activeWeapon->power * sin(activeWeapon->angle) + activeWeapon->gravitatin;
		//	activeWeapon->gravitatin += 0.05;
		//}
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

Weapon * PopWeapon(Weapon **headWeapons)
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
void InitTopPanels(PlayerTopPanel topPanels[])
{
	// Panel of Player 1
	topPanels[0].rect = { 0, 0, (int)0.3*SCREEN_WIDTH, (int)0.15*SCREEN_HEIGHT };
	// Panel of Player 2
	topPanels[1].rect = { SCREEN_WIDTH - topPanels[0].rect.w, topPanels[0].rect.y, topPanels[0].rect.w, topPanels[0].rect.h };
}

void DrawTopPanels(SDL_Renderer *renderer, PlayerTopPanel topPanels[], SDL_Texture * texturePanelName, SDL_Texture * texturePanelScore, SDL_Texture * texturePanelWordScore, int i)
{
	int Name_x = 0, Name_y = 0;
	int Text_Width = SCREEN_WIDTH / 8;
	int Text_Height = (2 * SCREEN_HEIGHT) / 15;
	if (i == 1)
	{
		Name_x = SCREEN_WIDTH - Text_Width - Name_x;
	}
	SDL_Rect Panel_rectUp = { Name_x, Name_y, Text_Width, Text_Height / 2 };
	SDL_Rect Panel_rectDown = { Name_x, Name_y + Panel_rectUp.h, Text_Width / 2, Text_Height / 2 };
	SDL_Rect Panel_rectWordScore = { Name_x + Text_Width / 2,Name_y + Panel_rectUp.h, Text_Width / 2, Text_Height / 2 };


	SDL_Texture* TexturePanel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, topPanels[i].rect.w, topPanels[i].rect.h);
	SDL_SetRenderTarget(renderer, TexturePanel);

	SDL_RenderCopy(renderer, texturePanelName, NULL, &Panel_rectUp);
	SDL_RenderCopy(renderer, texturePanelScore, NULL, &Panel_rectWordScore);
	SDL_RenderCopy(renderer, texturePanelWordScore, NULL, &Panel_rectDown);
	SDL_SetRenderTarget(renderer, NULL);
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

void CreateAndDrawTopPanels(SDL_Renderer *renderer, TTF_Font *font, Player players[], PlayerTopPanel topPanels[])
{
	SDL_Color fg = { 0, 0, 0 };
	SDL_Color bg = { 0, 102, 0 };
	SDL_Color colorGold = { 255, 215, 0 };
	for (int i = 0; i <= 1; i++)
	{
		//Creating Textures For Names
		SDL_Texture * texturePanelName = CreateTextureFromText(renderer, font, players[i].name, fg, bg);
		//Creating Texture "Score"
		SDL_Texture * texturePanelWordScore = CreateTextureFromText(renderer, font, "Score: ", colorGold, bg);
		//Creating Textures for Scores.
		SDL_Texture * texturePanelScore = CreateTextureFromNumber(renderer, font, players[i].score, colorGold, bg);

		DrawTopPanels(renderer, topPanels, texturePanelName, texturePanelScore, texturePanelWordScore, i);
		//Destroying Textures
		SDL_DestroyTexture(texturePanelName);
		SDL_DestroyTexture(texturePanelScore);
		SDL_DestroyTexture(texturePanelWordScore);
	}
}

void DrawBottomPanel(SDL_Renderer *renderer, SDL_Texture* TextureBottomPanel, SDL_Texture * ButtonUp, SDL_Texture * ButtonDown, SDL_Texture * ButtonLeft, SDL_Texture * ButtonRight, SDL_Texture * textureGunName, SDL_Texture * textureGunAngle, SDL_Texture * textureGunPuissance, int i)
{
	int Button1_x, Button1_y, Button1_w, Button1_h, Button2_x, Button2_y, Button2_w, Button2_h, Button3_x, Button3_y, Button3_w, Button3_h;
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

	SDL_RenderCopy(renderer, ButtonUp, NULL, &Button1Up_rect);
	SDL_RenderCopy(renderer, textureGunName, NULL, &Gun_rect);
	SDL_RenderCopy(renderer, ButtonDown, NULL, &Button1Down_rect);

	SDL_RenderCopy(renderer, ButtonUp, NULL, &Button2Up_rect);
	SDL_RenderCopy(renderer, textureGunAngle, NULL, &Angle_rect);
	SDL_RenderCopy(renderer, ButtonDown, NULL, &Button2Down_rect);

	SDL_RenderCopy(renderer, ButtonLeft, NULL, &Button3Left_rect);
	SDL_RenderCopy(renderer, textureGunPuissance, NULL, &Power_rect);
	SDL_RenderCopy(renderer, ButtonRight, NULL, &Button3Right_rect);
}

void CreateAndDrawBottomPanels(SDL_Renderer *renderer, TTF_Font *font, Player players[])
{
	SDL_Color fg = { 0, 0, 0 };
	SDL_Color bg = { 0,102,0 };

	//Creating Texture For Bottom Panel's Background
	SDL_Texture * textureBottomPanelBackground = LoadTexture(renderer, "Sprites/Green_Background_Camouflage_BottomPanel.BMP");

	SDL_Texture * ButtonUp = LoadTexture(renderer, "Sprites/Green_Arrow_Up.BMP");
	SDL_Texture * ButtonDown = LoadTexture(renderer, "Sprites/Green_Arrow_Down.BMP");
	SDL_Texture * ButtonLeft = LoadTexture(renderer, "Sprites/Green_Arrow_Left.BMP");
	SDL_Texture * ButtonRight = LoadTexture(renderer, "Sprites/Green_Arrow_Right.BMP");
	SDL_Texture * textureEricAndHank = LoadTexture(renderer, "Sprites/EricAndHank.BMP");

	SDL_Texture* textureBottomPanel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_Rect BottomPanel_rect = { 0, (int)(80 * SCREEN_HEIGHT) / 100, SCREEN_WIDTH, (int)(20 * SCREEN_WIDTH) / 100 };
	SDL_RenderCopy(renderer, textureBottomPanelBackground, NULL, &BottomPanel_rect);

	SDL_Rect EricAndHank_rect = { (int)((40 + 2.5)*SCREEN_WIDTH) / 100, (int)(80 * SCREEN_HEIGHT) / 100, (int)(15 * SCREEN_WIDTH) / 100, (int)(20 * SCREEN_HEIGHT) / 100 };
	SDL_RenderCopy(renderer, textureEricAndHank, NULL, &EricAndHank_rect);

	SDL_Texture *textureGunName = NULL;
	SDL_Texture * textureGunPuissance = NULL;

	for (int i = 0; i <= 1; i++)
	{
		// œÓÒÎÂ ‚˚ÒÚÂÎ‡ ÔÓÒÎÂ‰ÌÂ„Ó ÓÛÊËˇ headWeapon = NULL, ÔÓ˝ÚÓÏÛ ÌÂ ÔÓÎÛ˜‡ÂÚÒˇ ÔÓÎÛ˜ËÚ¸ Ì‡Á‚‡ÌËÂ ÓÛÊËˇ, ËÁ-Á‡ ˝ÚÓ„Ó ÔÓËÒıÓ‰ËÚ Ó¯Ë·Í‡.

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
			//Creating Texture For Gun's Name
			textureGunName = CreateTextureFromText(renderer, font, players[i].headWeapon->name, fg, bg);
			//Creating Texture For Gun's Puissance
			textureGunPuissance = CreateTextureFromNumber(renderer, font, players[i].power, fg, bg); //
		}
		else
		{
			//Creating Texture For Gun's Name
			textureGunName = CreateTextureFromText(renderer, font, "Empty", fg, bg);
			//Creating Texture For Gun's Puissance
			textureGunPuissance = CreateTextureFromNumber(renderer, font, 0, fg, bg);
		}

		//Creating Texture For Gun's Angle
		SDL_Texture * textureGunAngle = NULL;
		if (i == 0) textureGunAngle = CreateTextureFromNumber(renderer, font, (-1)*players[i].tank.cannon.angle, fg, bg);
		else textureGunAngle = CreateTextureFromNumber(renderer, font, players[i].tank.cannon.angle, fg, bg);

		DrawBottomPanel(renderer, textureBottomPanel, ButtonUp, ButtonDown, ButtonLeft, ButtonRight, textureGunName, textureGunAngle, textureGunPuissance, i);
		//Destroying Textures
		SDL_DestroyTexture(textureGunName);
		SDL_DestroyTexture(textureGunAngle);
		SDL_DestroyTexture(textureGunPuissance);
	}
	SDL_SetRenderTarget(renderer, textureBottomPanel);
	SDL_DestroyTexture(ButtonUp);
	SDL_DestroyTexture(ButtonDown);
	SDL_DestroyTexture(ButtonLeft);
	SDL_DestroyTexture(ButtonRight);
	SDL_DestroyTexture(textureEricAndHank);
	SDL_DestroyTexture(textureBottomPanelBackground);
	SDL_SetRenderTarget(renderer, NULL);

	//SDL_DestroyTexture(textureBottomPanel);
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