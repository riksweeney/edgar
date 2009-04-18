#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"

#include "bat.h"
#include "chicken.h"
#include "rock.h"
#include "boulder_boss.h"
#include "spider.h"
#include "grub.h"
#include "grub_boss.h"
#include "fireball.h"
#include "wasp.h"
#include "small_boulder.h"
#include "red_grub.h"

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
{"enemy/red_grub", &addRedGrub}
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
