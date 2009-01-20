#include "enemies.h"

extern Entity *addBat(int, int);
extern Entity *addChicken(int, int);

static Constructor enemies[] = {
{"enemy/bat", &addBat},
{"enemy/chicken", &addChicken},
{"enemy/rat", &addBat}};

static int length = sizeof(enemies) / sizeof(Constructor);

Entity *addEnemy(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(enemies[i].name, name) == 0)
		{
			printf("Adding Enemy %s to %d %d\n", enemies[i].name, x, y);

			return enemies[i].construct(x, y);
		}
	}

	printf("Could not find enemy %s\n", name);

	exit(1);
}
