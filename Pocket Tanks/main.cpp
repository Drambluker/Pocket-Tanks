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

		/* ������� ������ */
		UpdateLogic(&scene);

		/* ��������� ����� */
		DoRender(&scene);
	}

	DestroyScene(&scene);

	return 0;
}