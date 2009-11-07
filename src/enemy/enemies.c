/*
Copyright (C) 2009 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"

#include "bat.h"
#include "chicken.h"
#include "rock.h"
#include "spider.h"
#include "grub.h"
#include "fireball.h"
#include "wasp.h"
#include "small_boulder.h"
#include "red_grub.h"
#include "spitting_plant.h"
#include "jumping_slime.h"
#include "egg.h"
#include "baby_slime.h"
#include "spinner.h"
#include "floating_snapper.h"
#include "enemy_generator.h"
#include "flying_bug.h"
#include "eye_stalk.h"
#include "snail.h"
#include "snail_shell.h"
#include "scorpion.h"
#include "laser_grid.h"
#include "ceiling_snapper.h"
#include "gazer.h"
#include "sludge.h"
#include "summoner.h"
#include "centurion.h"
#include "armadillo.h"
#include "ceiling_crawler.h"
#include "tortoise.h"
#include "fire_burner.h"
#include "energy_drainer.h"
#include "book.h"
#include "spike_wall.h"

#include "../boss/ant_lion.h"
#include "../boss/boulder_boss.h"
#include "../boss/grub_boss.h"
#include "../boss/golem_boss.h"
#include "../boss/snake_boss.h"
#include "../boss/fly_boss.h"
#include "../boss/flying_maggot.h"
#include "../boss/golem_rock_dropper.h"
#include "../boss/blob_boss.h"
#include "../system/error.h"

static Constructor enemies[] = {
{"enemy/bat", &addBat},
{"enemy/red_bat", &addBat},
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
{"enemy/bee_hive", &addEnemyGenerator},
{"enemy/flying_bug", &addFlyingBug},
{"boss/fly_boss", &addFlyBoss},
{"boss/ant_lion", &addAntLion},
{"enemy/eye_stalk", &addEyeStalk},
{"enemy/snail", &addSnail},
{"enemy/purple_snail", &addSnail},
{"enemy/purple_snail_shell", &addSnailShell},
{"enemy/snail_shell", &addSnailShell},
{"boss/flying_maggot", &addFlyingMaggot},
{"boss/flying_maggot_2", &addFlyingMaggot},
{"enemy/scorpion", &addScorpion},
{"enemy/laser_grid", &addLaserGrid},
{"enemy/horizontal_laser_grid", &addLaserGrid},
{"enemy/ceiling_snapper", &addCeilingSnapper},
{"enemy/gazer", &addGazer},
{"enemy/sludge", &addSludge},
{"boss/golem_rock_dropper", &addGolemRockDropper},
{"enemy/summoner", &addSummoner},
{"enemy/centurion", &addCenturion},
{"enemy/armadillo", &addArmadillo},
{"enemy/ceiling_crawler", &addCeilingCrawler},
{"enemy/tortoise", &addTortoise},
{"enemy/fire_burner", &addFireBurner},
{"boss/blob_boss_1", &addBlobBoss},
{"boss/blob_boss_2", &addBlobBoss},
{"enemy/energy_drainer", &addEnergyDrainer},
{"enemy/red_book", &addBook},
{"enemy/green_book", &addBook},
{"enemy/yellow_book", &addBook},
{"enemy/blue_book", &addBook},
{"enemy/spike_wall", &addSpikeWall}
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

			if (e->pain == NULL)
			{
				e->pain = &enemyPain;
			}

			return e;
		}
	}

	showErrorAndExit("Could not find enemy %s", name);

	return NULL;
}
