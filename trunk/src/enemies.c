#include "enemies.h"

extern void addBat(int, int);

static Enemy enemies[] = {
{"bat", &addBat},
{"spider", &addBat},
{"chicken", &addBat},
{"rat", &addBat}};

static int length = sizeof(enemies) / sizeof(Enemy);

void addEnemy(char *name, int x, int y)
{
	int i;
	
	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(enemies[i].name, name) == 0)
		{
			printf("Adding Enemy %s to %d %d\n", enemies[i].name, x, y);
			
			enemies[i].construct(x, y);
		}
	}
}
