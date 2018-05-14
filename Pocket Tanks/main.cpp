#include "functions.h"
#include "cmath"

#define DEBUG 1

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
#if DEBUG == 1
	RecordRow records[NUMBER_OF_RECORD_ROWS];
	LoadRecords(records);
	printf_s("\nRECORDS\n");

	for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
	{
		printf_s("%d) %s | Score: %d\n", i + 1, records[i].name, records[i].score);
	}
#endif // DEBUG == 1
	DestroyScene(&scene);

	return 0;
}