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
#include "spitting_plant.h"
#include "jumping_slime.h"
#include "egg.h"
#include "golem_boss.h"
#include "baby_slime.h"
#include "spinner.h"
#include "snake_boss.h"
#include "floating_snapper.h"

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
{"enemy/red_grub", &addRedGrub},
{"enemy/spitting_plant", &addSpittingPlant},
{"enemy/jumping_slime", &addJumpingSlime},
{"enemy/purple_jumping_slime", &addJumpingSlime},
{"enemy/jumping_slime_egg", &addEgg},
{"boss/golem_boss", &addGolemBoss},
{"enemy/baby_slime", &addBabySlime},
{"enemy/red_baby_slime", &addBabySlime},
{"enemy/eye_spinner", &addSpinner},
{"boss/snake_boss", &addSnakeBoss},
{"enemy/floating_snapper", &addFloatingSnapper},
};

static int length = sizeof(enemies) / sizeof(Constructor);

Entity *addEnemy(char *name, int x, int y)
{
	int i;
	Entity *e;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(enemies[i].name, name) == 0)
		{
			e = enemies[i].construct(x, y, name);

			if (e->fallout == NULL)
			{
				e->fallout = &entityDie;
			}

			return e;
		}
	}

	printf("Could not find enemy %s\n", name);

	exit(1);
}
