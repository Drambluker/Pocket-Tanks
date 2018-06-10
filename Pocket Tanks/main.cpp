#include "functions.h"

int main(int argc, char** argv)
{
	Scene scene;
	LoadGame(&scene);

	bool quit = false;

	while (!quit)
	{
		quit = ProcessEvents(&scene);

		/* Игровая логика */
		UpdateLogic(&scene);

		/* Отрисовка сцены */
		DoRender(&scene);
	}

	DestroyGame(&scene);

	return 0;
}