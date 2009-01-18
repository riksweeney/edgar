#include "enemies.h"

extern void addBat(int, int);
extern void addChicken(int, int);

static Special enemies[] = {
{"enemy/bat", &addBat},
{"enemy/chicken", &addChicken},
{"enemy/rat", &addBat}};

static int length = sizeof(enemies) / sizeof(Special);

void addEnemy(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(enemies[i].name, name) == 0)
		{
			printf("Adding Enemy %s to %d %d\n", enemies[i].name, x, y);

			enemies[i].construct(x, y);
			
			return;
		}
	}
}
