#include "functions.h"
#include "cmath"

int main(int argc, char** argv)
{
	Scene scene;
	LoadScene(&scene);

	bool quit = false;

	while (!quit)
	{
		quit = ProcessEvents(&scene);

		/* Игровая логика */
		UpdateLogic(&scene);

		/* Отрисовка сцены */
		DoRender(&scene);
	}

	DestroyScene(&scene);

	return 0;
}