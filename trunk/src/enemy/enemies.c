/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "lightning_tortoise.h"
#include "fire_burner.h"
#include "energy_drainer.h"
#include "book.h"
#include "spike_wall.h"
#include "whirlwind.h"
#include "poltergiest.h"
#include "large_book.h"
#include "spike_sphere.h"
#include "exploding_spike_sphere.h"
#include "thunder_cloud.h"
#include "spirit.h"
#include "large_spider.h"
#include "huge_spider.h"
#include "mouth_stalk.h"
#include "ground_spear.h"
#include "dragon_fly.h"
#include "auto_spike_ball.h"
#include "gold_centurion.h"
#include "scanner.h"
#include "fish.h"
#include "fire_tortoise.h"
#include "dark_summoner.h"
#include "pendulum.h"
#include "ice_tortoise.h"
#include "splitter.h"
#include "buzz_saw.h"
#include "ground_snapper.h"
#include "sasquatch.h"

#include "../boss/ant_lion.h"
#include "../boss/boulder_boss.h"
#include "../boss/grub_boss.h"
#include "../boss/golem_boss.h"
#include "../boss/snake_boss.h"
#include "../boss/fly_boss.h"
#include "../boss/flying_maggot.h"
#include "../boss/golem_rock_dropper.h"
#include "../boss/blob_boss.h"
#include "../boss/black_book.h"
#include "../boss/mataeus.h"
#include "../boss/mataeus_wall.h"
#include "../boss/boulder_boss_2.h"
#include "../boss/armour_boss.h"
#include "../boss/awesome_boss.h"
#include "../boss/awesome_boss_meter.h"
#include "../boss/evil_edgar.h"
#include "../boss/borer_boss.h"
#include "../boss/phoenix.h"
#include "../boss/sorceror.h"
#include "../boss/centurion_boss.h"
#include "../boss/sewer_boss.h"

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
{"enemy/tortoise", &addLightningTortoise},
{"enemy/fire_burner", &addFireBurner},
{"boss/blob_boss_1", &addBlobBoss},
{"boss/blob_boss_2", &addBlobBoss},
{"enemy/energy_drainer", &addEnergyDrainer},
{"enemy/red_book", &addBook},
{"enemy/green_book", &addBook},
{"enemy/yellow_book", &addBook},
{"enemy/blue_book", &addBook},
{"enemy/spike_wall", &addSpikeWall},
{"enemy/whirlwind", &addWhirlwind},
{"enemy/poltergiest_1", &addPoltergiest},
{"enemy/poltergiest_2", &addPoltergiest},
{"enemy/poltergiest_3", &addPoltergiest},
{"enemy/poltergiest_4", &addPoltergiest},
{"enemy/large_red_book", &addLargeBook},
{"enemy/large_blue_book", &addLargeBook},
{"enemy/large_yellow_book", &addLargeBook},
{"enemy/large_green_book", &addLargeBook},
{"enemy/spike_sphere", &addSpikeSphere},
{"enemy/exploding_spike_sphere", &addExplodingSpikeSphere},
{"enemy/thunder_cloud", &addThunderCloud},
{"boss/black_book_1", &addBlackBook},
{"boss/mataeus", &addMataeus},
{"enemy/spirit", &addSpirit},
{"boss/mataeus_wall", &addMataeusWall},
{"boss/boulder_boss_2", &addBoulderBoss2},
{"enemy/large_spider", &addLargeSpider},
{"enemy/large_red_spider", &addLargeSpider},
{"enemy/red_sludge", &addSludge},
{"enemy/huge_spider", &addHugeSpider},
{"boss/armour_boss", &addArmourBoss},
{"enemy/mouth_stalk", &addMouthStalk},
{"enemy/ground_spear", &addGroundSpear},
{"enemy/dragon_fly", &addDragonFly},
{"enemy/auto_spike_ball", &addAutoSpikeBall},
{"enemy/gold_centurion", &addGoldCenturion},
{"enemy/green_scanner", &addScanner},
{"enemy/blue_scanner", &addScanner},
{"boss/awesome_boss_1", &addAwesomeBoss},
{"boss/awesome_boss_2", &addAwesomeBoss},
{"boss/awesome_boss_3", &addAwesomeBoss},
{"boss/awesome_boss_4", &addAwesomeBoss},
{"boss/awesome_boss_meter", &addAwesomeBossMeter},
{"boss/evil_edgar", &addEvilEdgar},
{"enemy/fish", &addFish},
{"enemy/green_fish", &addFish},
{"boss/borer_boss", &addBorerBoss},
{"enemy/fire_tortoise", &addFireTortoise},
{"enemy/dark_summoner", &addDarkSummoner},
{"enemy/red_centurion", &addCenturion},
{"boss/phoenix", &addPhoenix},
{"enemy/pendulum", &addPendulum},
{"enemy/ice_tortoise", &addIceTortoise},
{"enemy/splitter", &addSplitter},
{"enemy/splitter_medium", &addSplitterMedium},
{"enemy/splitter_small", &addSplitterSmall},
{"enemy/buzz_saw", &addBuzzSaw},
{"boss/sorceror", &addSorceror},
{"boss/centurion_boss", &addCenturionBoss},
{"boss/sewer_boss", &addSewerBoss},
{"enemy/ground_snapper", &addGroundSnapper},
{"enemy/sasquatch", &addSasquatch}
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

			e->originalWeight = e->weight;

			return e;
		}
	}

	showErrorAndExit("Could not find enemy %s", name);

	return NULL;
}
