#include "headers.h"

#include "enemy/bat.h"
#include "enemy/chicken.h"
#include "enemy/rock.h"
#include "enemy/boulder_boss.h"
#include "enemy/spider.h"
#include "enemy/grub.h"
#include "enemy/grub_boss.h"
#include "enemy/fireball.h"
#include "enemy/wasp.h"
#include "enemy/small_boulder.h"
#include "villager.h"

static Constructor enemies[] = {
{"enemy/bat", &addBat},
{"enemy/chicken", &addChicken},
{"common/large_rock", &addLargeRock},
{"boss/boulder_boss", &addBoulderBoss},
{"enemy/spider", &addSpider},
{"enemy/red_spider", &addSpider},
{"enemy/grub", &addGrub},
{"boss/grub_boss", &addGrubBoss},
{"enemy/jumping_fireball", &addJumpingFireball},
{"enemy/wasp", &addWasp},
{"common/small_rock", &addSmallRock},
{"enemy/small_boulder", &addSmallBoulder},
{"enemy/villager", &addVillager}
};

static int length = sizeof(enemies) / sizeof(Constructor);

Entity *addEnemy(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(enemies[i].name, name) == 0)
		{
			return enemies[i].construct(x, y, name);
		}
	}

	printf("Could not find enemy %s\n", name);

	exit(1);
}
