#include "temp.h"

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

//void DrawTopPanels(SDL_Renderer *renderer, PlayerTopPanel topPanels[], SDL_Texture * texturePanelName, SDL_Texture * texturePanelScore, SDL_Texture * texturePanelWordScore, int i)
//{
//	int Name_x = 0, Name_y = 0;
//	int Text_Width = SCREEN_WIDTH / 8;
//	int Text_Height = (2 * SCREEN_HEIGHT) / 15;
//	if (i == 1) Name_x = SCREEN_WIDTH - Text_Width - Name_x;
//	SDL_Rect Name_rect = { Name_x, Name_y, Text_Width, Text_Height / 2 };
//	SDL_Rect WordScore_rect = { Name_x, Name_y + Name_rect.h, Text_Width / 2, Text_Height / 2 };
//	SDL_Rect Score_rect = { Name_x + Text_Width / 2,Name_y + Name_rect.h, Text_Width / 2, Text_Height / 2 };
//
//	SDL_Texture* TexturePanel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_TARGET, topPanels[i].rect.w, topPanels[i].rect.h);
//	SDL_SetRenderTarget(renderer, TexturePanel); 
//
//	SDL_RenderCopy(renderer, texturePanelName, NULL, &Name_rect);
//	SDL_RenderCopy(renderer, texturePanelWordScore, NULL, &WordScore_rect);
//	SDL_RenderCopy(renderer, texturePanelScore, NULL, &Score_rect);
//	SDL_SetRenderTarget(renderer, NULL);
//	SDL_DestroyTexture(TexturePanel);
//	TexturePanel = NULL;
//
//	SDL_DestroyTexture(texturePanelName);
//	SDL_DestroyTexture(texturePanelWordScore);
//	SDL_DestroyTexture(texturePanelScore);
//	texturePanelName = NULL;
//	texturePanelScore = NULL;
//	texturePanelWordScore = NULL;
//}

void DrawMsgOnBottonScreen(Scene *scene, const char *Msg)
{
	SDL_Color colorBlack = { 0, 0, 0 };
	SDL_Color colorBg = { 153, 153, 0 };
	SDL_Rect WordCurrentPlayerChoosing_rect = { (int)((27.0 / 80)*SCREEN_WIDTH), (int)((14.0 / 15)*SCREEN_HEIGHT) + 10, (int)(SCREEN_WIDTH / 7.0), (int)(SCREEN_HEIGHT / 15.0) - 10 };
	SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
	SDL_RenderFillRect(scene->renderer, &WordCurrentPlayerChoosing_rect);
	SDL_SetRenderDrawColor(scene->renderer, 0, 0, 0, 0);
	SDL_Texture * textureCurrentPlayerChoosing = CreateTextureFromText(scene->renderer, scene->font, Msg, colorBlack, colorBg);
	SDL_RenderCopy(scene->renderer, textureCurrentPlayerChoosing, NULL, &WordCurrentPlayerChoosing_rect);
	SDL_DestroyTexture(textureCurrentPlayerChoosing);
}

/*
//There is a closing } but no opening {, This is why there is a signal of error on this code, it's just a part(end) of a a function CreateAndDraw2PlayersMenu().
while (scene->quitThirdScreen != true)
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
//Event for landscape
DrawMsgOnBottonScreen(scene, "Choose Landscape...");
if ((scene->event.button.x >= Landscape1_rect.x) && (scene->event.button.x <= Landscape1_rect.x + Landscape1_rect.w)
&& (scene->event.button.y >= Landscape1_rect.y) && (scene->event.button.y <= Landscape1_rect.y + Landscape1_rect.h))
{
scene->landscapeType = 1;
InitLandscape(&scene->landscape, &scene->defaultLandscape, scene->landscapeType);
}
else
{
if ((scene->event.button.x >= Landscape2_rect.x) && (scene->event.button.x <= Landscape2_rect.x + Landscape2_rect.w)
&& (scene->event.button.y >= Landscape2_rect.y) && (scene->event.button.y <= Landscape2_rect.y + Landscape2_rect.h))
{
scene->landscapeType = 2;
InitLandscape(&scene->landscape, &scene->defaultLandscape, scene->landscapeType);
}
if ((scene->event.button.x >= Landscape3_rect.x) && (scene->event.button.x <= Landscape3_rect.x + Landscape3_rect.w)
&& (scene->event.button.y >= Landscape3_rect.y) && (scene->event.button.y <= Landscape3_rect.y + Landscape3_rect.h))
{
scene->landscapeType = 3;
InitLandscape(&scene->landscape, &scene->defaultLandscape, scene->landscapeType);
}
if ((scene->event.button.x >= Landscape4_rect.x) && (scene->event.button.x <= Landscape4_rect.x + Landscape4_rect.w)
&& (scene->event.button.y >= Landscape4_rect.y) && (scene->event.button.y <= Landscape4_rect.y + Landscape4_rect.h))
{
scene->landscapeType = 4;
InitLandscape(&scene->landscape, &scene->defaultLandscape, scene->landscapeType);
}

}



//Event for name of player 1
if ((scene->event.button.x >= Blanc1_rect.x) && (scene->event.button.x <= Blanc1_rect.x + Blanc1_rect.w)
&& (scene->event.button.y >= Blanc1_rect.y) && (scene->event.button.y <= Blanc1_rect.y + Blanc1_rect.h))
{
SDL_SetRenderDrawColor(scene->renderer, 255, 255, 255, 0);
SDL_RenderFillRect(scene->renderer, &Blanc1_rect);

scene->players[0].name = "";
char *Text = "";
int index = 0;
bool done = false;
while (done != true) {
do {
SDL_StartTextInput();
while (SDL_PollEvent(&scene->event))
{
switch (scene->event.type)
{
case SDL_QUIT: ExitWhileInMenu(scene); break;
case SDL_TEXTINPUT:

int r = strcat_s(Text, sizeof(char), scene->event.text.text);
printf_s("Name of player %d = %c r = %d", scene->playerLap, Text, r);
if (r != 0) printf_s("Error while joining the letter to the name");
SDL_SetRenderDrawColor(scene->renderer, 0, 0, 0, 0);
SDL_Rect Symbol_rect = { Blanc1_rect.x + index * (int)(Blanc1_rect.w / 10.0 ) + 1, Blanc1_rect.y + 1, (int)(Blanc1_rect.w / 10.0) - 3, Blanc1_rect.h - 2 };
SDL_Texture * textureSymbol = CreateTextureFromText(scene->renderer, scene->font, scene->event.text.text, colorBlack, colorWhite);
SDL_RenderCopy(scene->renderer, textureSymbol, NULL, &Symbol_rect);
SDL_RenderPresent(scene->renderer);
SDL_DestroyTexture(textureSymbol);
//SDL_RenderPresent(scene->renderer);
index++;
break;
}
}
SDL_StopTextInput();
} while (!(scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT &&
((scene->event.button.x >= Blanc1_rect.x) && (scene->event.button.x <= Blanc1_rect.x + Blanc1_rect.w) &&
(scene->event.button.y >= Blanc1_rect.y) && (scene->event.button.y <= Blanc1_rect.y + Blanc1_rect.h)))
&& index < 10);
done = true;
}
scene->players[0].name = Text;
}
//Event for name of player 2
if ((scene->event.button.x >= Blanc2_rect.x) && (scene->event.button.x <= Blanc2_rect.x + Blanc2_rect.w)
&& (scene->event.button.y >= Blanc2_rect.y) && (scene->event.button.y <= Blanc2_rect.y + Blanc2_rect.h))
{
SDL_SetRenderDrawColor(scene->renderer, 255, 255, 255, 0);
SDL_RenderFillRect(scene->renderer, &Blanc2_rect);

scene->players[1].name = "";
char *Text = "";
int index = 0;
bool done = false;
while (done != true) {
do {
SDL_StartTextInput();
while (SDL_PollEvent(&scene->event))
{
switch (scene->event.type)
{
case SDL_QUIT: ExitWhileInMenu(scene); break;
case SDL_TEXTINPUT:
int r = strcat_s(Text, sizeof(char), scene->event.text.text);
if (r != 0) printf_s("Error while joining the letter to the name");
SDL_SetRenderDrawColor(scene->renderer, 0, 0, 0, 0);
SDL_Rect Symbol_rect = { Blanc2_rect.x + index * (int)(Blanc2_rect.w / 10.0) + 1, Blanc2_rect.y + 1, (int)(Blanc2_rect.w / 10.0) - 3, Blanc2_rect.h - 2 };
SDL_Texture * textureSymbol = CreateTextureFromText(scene->renderer, scene->font, scene->event.text.text, colorBlack, colorWhite);
SDL_RenderCopy(scene->renderer, textureSymbol, NULL, &Symbol_rect);
SDL_RenderPresent(scene->renderer);
SDL_DestroyTexture(textureSymbol);
//SDL_RenderPresent(scene->renderer);
index++;
break;
}
}
SDL_StopTextInput();
} while (!(scene->event.type == SDL_MOUSEBUTTONDOWN && scene->event.button.button == SDL_BUTTON_LEFT &&
((scene->event.button.x >= Blanc2_rect.x) && (scene->event.button.x <= Blanc2_rect.x + Blanc2_rect.w) &&
(scene->event.button.y >= Blanc2_rect.y) && (scene->event.button.y <= Blanc2_rect.y + Blanc2_rect.h)))
&& index < 10);
done = true;
}
scene->players[1].name = Text;
}


////Event for the Weapons
DrawMsgOnBottonScreen(scene, "Choose Weapons...");
//If Cliked on Lolly Bomb
if ((scene->event.button.x >= LollyBomb_rect.x) && (scene->event.button.x <= LollyBomb_rect.x + LollyBomb_rect.w)
&& (scene->event.button.y >= LollyBomb_rect.y) && (scene->event.button.y <= LollyBomb_rect.y + LollyBomb_rect.h) && Choosed_LollyBomb != true)
{
Weapon *weapon = NULL;
weapon = (Weapon *)malloc(sizeof(Weapon));
weapon->name = "Lolly Bomb";
weapon->score = 1;
weapon->angle = 0;
weapon->gravitatin = 0;
weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
//PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);
Choosed_LollyBomb = true;
SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
SDL_RenderFillRect(scene->renderer, &LollyBomb_rect);
//SDL_RenderPresent(scene->renderer);
}
//If Cliked on Laser
if ((scene->event.button.x >= Laser_rect.x) && (scene->event.button.x <= Laser_rect.x + Laser_rect.w)
&& (scene->event.button.y >= Laser_rect.y) && (scene->event.button.y <= Laser_rect.y + Laser_rect.h) && Choosed_Laser != true)
{
Weapon *weapon = NULL;
weapon = (Weapon *)malloc(sizeof(Weapon));
weapon->name = "Laser";
weapon->score = 5;
weapon->angle = 0;
weapon->gravitatin = 0;
weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
//PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);
Choosed_Laser = true;
SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
SDL_RenderFillRect(scene->renderer, &Laser_rect);
//SDL_RenderPresent(scene->renderer);

}
//If Cliked on Ravine
if ((scene->event.button.x >= Ravine_rect.x) && (scene->event.button.x <= Ravine_rect.x + Ravine_rect.w)
&& (scene->event.button.y >= Ravine_rect.y) && (scene->event.button.y <= Ravine_rect.y + Ravine_rect.h) && Choosed_Ravine != true)
{
Weapon *weapon = NULL;
weapon = (Weapon *)malloc(sizeof(Weapon));
weapon->name = "Ravine";
weapon->score = 0;
weapon->angle = 0;
weapon->gravitatin = 0;
weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
//PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);
Choosed_Ravine = true;
SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
SDL_RenderFillRect(scene->renderer, &Ravine_rect);
//SDL_RenderPresent(scene->renderer);
}
//If Cliked on Lolly Bomb 2.0
if ((scene->event.button.x >= LollyBomb2_rect.x) && (scene->event.button.x <= LollyBomb2_rect.x + LollyBomb2_rect.w)
&& (scene->event.button.y >= LollyBomb2_rect.y) && (scene->event.button.y <= LollyBomb2_rect.y + LollyBomb2_rect.h) && Choosed_LollyBomb2 != true)
{
Weapon *weapon = NULL;
weapon = (Weapon *)malloc(sizeof(Weapon));
weapon->name = "Lolly Bomb 2.0";
weapon->score = 2;
weapon->angle = 0;
weapon->gravitatin = 0;
weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
//PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);
Choosed_LollyBomb2 = true;
SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
SDL_RenderFillRect(scene->renderer, &LollyBomb2_rect);
//SDL_RenderPresent(scene->renderer);
}
//If Cliked on Chinese Wall
if ((scene->event.button.x >= ChineseWall_rect.x) && (scene->event.button.x <= ChineseWall_rect.x + ChineseWall_rect.w)
&& (scene->event.button.y >= ChineseWall_rect.y) && (scene->event.button.y <= ChineseWall_rect.y + ChineseWall_rect.h) && Choosed_ChineseWall != true)
{
Weapon *weapon = NULL;
weapon = (Weapon *)malloc(sizeof(Weapon));
weapon->name = "Chinese Wall";
weapon->score = 0;
weapon->angle = 0;
weapon->gravitatin = 0;
weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
//PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);
Choosed_ChineseWall = true;
SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
SDL_RenderFillRect(scene->renderer, &ChineseWall_rect);
//SDL_RenderPresent(scene->renderer);
}
//If Cliked on Pineaple
if ((scene->event.button.x >= Pineaple_rect.x) && (scene->event.button.x <= Pineaple_rect.x + Pineaple_rect.w)
&& (scene->event.button.y >= Pineaple_rect.y) && (scene->event.button.y <= Pineaple_rect.y + Pineaple_rect.h) && Choosed_Pineaple != true)
{
Weapon *weapon = NULL;
weapon = (Weapon *)malloc(sizeof(Weapon));
weapon->name = "Pineaple";
weapon->score = 4;
weapon->angle = 0;
weapon->gravitatin = 0;
weapon->texture = LoadTexture(scene->renderer, "Sprites/weapon1.bmp");
PushWeapon(weapon, &scene->players[scene->playerLap - 1].headWeapon, &scene->players[scene->playerLap - 1].tailWeapon);
//PushWeapon(weapon, &(scene->players)[scene->playerLap - 1].headWeapon, &(scene->players)[scene->playerLap - 1].tailWeapon);
Choosed_Pineaple = true;
SDL_SetRenderDrawColor(scene->renderer, 153, 153, 0, 0);
SDL_RenderFillRect(scene->renderer, &Pineaple_rect);
//SDL_RenderPresent(scene->renderer);
}

}


if ((strcmp(scene->players[0].name, "None") != 0) && (strcmp(scene->players[1].name, "None") != 0) && (scene->landscapeType ==1 || scene->landscapeType == 2 || scene->landscapeType == 3 ||
scene->landscapeType == 4)  && Choosed_LollyBomb == true && Choosed_Laser == true && Choosed_Ravine == true && Choosed_LollyBomb2 == true && Choosed_ChineseWall == true && Choosed_Pineaple == true)
{
//Change the color of the button "Play" and check for the click on it.
DrawMsgOnBottonScreen(scene, "Press Play...");
SDL_Color colorButtonPlay2 = { 0, 102, 0 };
SDL_SetRenderDrawColor(scene->renderer, 0, 102, 0, 0);
SDL_RenderFillRect(scene->renderer, &ButtonPlay_rect);
SDL_Texture * textureButtonPlay2 = CreateTextureFromText(scene->renderer, scene->font, "Play", colorBlack, colorButtonPlay2);
SDL_RenderCopy(scene->renderer, textureButtonPlay2, NULL, &ButtonPlay_rect);
SDL_RenderPresent(scene->renderer);
SDL_DestroyTexture(textureButtonPlay);
//SDL_RenderPresent(scene->renderer);

scene->ClickOnPlay = false;
while (scene->ClickOnPlay != true)
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
scene->ClickOnPlay = true;
scene->quitThirdScreen = true;
scene->quitSecondScreen = true;
break;
}
}
}
}
break;

}
else {
if (scene->playerLap == 1) scene->playerLap = 2;
else scene->playerLap = 1;

}
}

SDL_SetRenderTarget(scene->renderer, NULL);
SDL_DestroyTexture(TextureMainMenuStart);
};
}
*/

/*void DrawBottomPanel(SDL_Renderer *renderer, SDL_Texture* TextureBottomPanel, SDL_Texture * ButtonUp, SDL_Texture * ButtonDown, SDL_Texture * ButtonLeft, SDL_Texture * ButtonRight, SDL_Texture * textureGunName, SDL_Texture * textureGunAngle, SDL_Texture * textureGunPuissance, int i)
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
*/