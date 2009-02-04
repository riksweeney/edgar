#include "headers.h"

#include "bat.h"
#include "chicken.h"
#include "rock.h"
#include "boulder_boss.h"

static Constructor enemies[] = {
{"enemy/bat", &addBat},
{"enemy/chicken", &addChicken},
{"common/large_rock", &addLargeRock},
{"boss/boulder_boss", &addBoulderBoss}
};

static int length = sizeof(enemies) / sizeof(Constructor);

Entity *addEnemy(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(enemies[i].name, name) == 0)
		{
			return enemies[i].construct(x, y);
		}
	}

	printf("Could not find enemy %s\n", name);

	exit(1);
}
