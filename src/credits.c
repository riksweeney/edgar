/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "headers.h"

#include "audio/music.h"
#include "collisions.h"
#include "credits.h"
#include "enemy/enemies.h"
#include "entity.h"
#include "game.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "inventory.h"
#include "item/key_items.h"
#include "map.h"
#include "player.h"
#include "system/error.h"
#include "system/load_save.h"
#include "system/pak.h"
#include "system/random.h"
#include "system/resources.h"
#include "world/target.h"

extern Game game;
extern Entity *self, player;

static Credits credits;
static char *enemies[] = {
			"edgar/edgar",
			"enemy/ghost",
			"enemy/chicken",
			"enemy/grub",
			"enemy/wasp",
			"enemy/red_grub",
			"enemy/jumping_slime",
			"enemy/purple_jumping_slime",
			"enemy/flying_bug",
			"enemy/scorpion",
			"enemy/snail",
			"enemy/purple_snail",
			"enemy/red_bat",
			"enemy/gazer",
			"enemy/energy_drainer",
			"enemy/sludge",
			"enemy/summoner",
			"enemy/centurion",
			"enemy/tortoise",
			"enemy/ceiling_crawler",
			"enemy/whirlwind",
			"enemy/large_blue_book",
			"enemy/large_red_book",
			"enemy/large_yellow_book",
			"enemy/large_green_book",
			"enemy/large_spider",
			"enemy/large_red_spider",
			"enemy/red_sludge",
			"enemy/dragon_fly",
			"enemy/green_scanner",
			"enemy/blue_scanner",
			"enemy/fire_tortoise",
			"enemy/dark_summoner",
			"enemy/ice_tortoise",
			"boss/centurion_boss",
			"enemy/splitter",
			"enemy/sasquatch",
			"enemy/yellow_centipede",
			"enemy/red_centipede",
			"enemy/green_centipede",
			"enemy/red_jumping_slime",
			"enemy/frog",
			"enemy/undead_gazer",
			"enemy/fly_trap",
			"enemy/armour_changer",
			"enemy/bat",
			"enemy/slug",
			"enemy/upside_down_slug",
			"enemy/arrow_skeleton",
			"enemy/sword_skeleton",
			"enemy/ceiling_creeper",
			"enemy/zombie",
			"enemy/master_tortoise" /* Always last */
};
static int enemiesLength = sizeof(enemies) / sizeof(char *);

static char *bosses[] = {
			"boss/grub_boss",
			"boss/golem_boss",
			"boss/fly_boss",
			"boss/snake_boss",
			"boss/gargoyle",
			"boss/blob_boss_2",
			"boss/armour_boss",
			"boss/awesome_boss_1",
			"boss/mataeus",
			"boss/phoenix",
			"boss/borer_boss",
			"boss/evil_edgar",
			"boss/sewer_boss",
			"boss/cave_boss",
			"boss/black_book_2",
			"boss/azriel",
			"boss/grimlore",
			"boss/sorceror_2"
};
static int bossesLength = sizeof(bosses) / sizeof(char *);

static char *bossNames[] = {
			"boss/grub_boss", "King Grub", "map03", "grub_boss_start",
			"boss/golem_boss", "The Golem", "map02", "golem_boss_start",
			"boss/fly_boss", "Queen Wasp", "map05", "fly_boss_start",
			"boss/snake_boss", "The Swamp Guardian", "map04", "snake_boss_start",
			"boss/gargoyle", "The Gargoyle", "map06", "gargoyle_start",
			"boss/blob_boss_2", "The Blob", "map10", "blob_boss_start",
			"boss/armour_boss", "The Watchdog", "map08", "boss/armour_boss",
			"boss/awesome_boss_1", "The Awesome Foursome", "map13", "awesome_boss_start",
			"boss/mataeus", "Mataeus", "map11", "mataeus_boss_start",
			"boss/phoenix", "The Phoenix", "map12", "lava_boss_start",
			"boss/borer_boss", "The Borer", "map14", "boss/borer_boss",
			"boss/evil_edgar", "Evil Edgar", "map10", "evil_edgar_start",
			"boss/sewer_boss", "The Sewer Dweller", "map07", "sewer_boss_start",
			"boss/cave_boss", "The Salamander", "map20", "cave_boss_start",
			"boss/black_book_2", "The Black Book", "map11", "boss/black_book_2",
			"boss/azriel", "Azriel", "map21", "boss/azriel",
			"boss/grimlore", "Grimlore", "map16", "hidden_passage_hint",
			"boss/sorceror_2", "The Sorceror", "map24", "boss/sorceror_2"
};

static void initCredits(void);
static Entity *loadCreditsEntity(char *);
static int getNextEntity(void);
static void doEndCredits(void);
static void doGameStats(void);
static void initGameStats(void);
static void drawEndCredits(void);
static void drawGameStats(void);
static void doDefeatedBosses(void);
static void initDefeatedBosses(void);
static int getNextBoss(void);
static void drawDefeatedBosses(void);
static Entity *loadCreditsBoss(char *);
static void bossMoveOffScreen(void);
static char *getBossName(char *);
static char *getBossMap(char *);
static char *getBossTrigger(char *);
static void doChaos(void);
static void drawChaos(void);
static void shuffleEnemies(void);
static void doEdgarLogo(void);
static void drawEdgarLogo(void);
static int doCreditsEntities(void);

void doCredits()
{
	switch (credits.status)
	{
		case 1:
			doGameStats();
		break;

		case 2:
			doDefeatedBosses();
		break;

		case 3:
			doChaos();
		break;

		case 4:
			doEdgarLogo();
		break;

		default:
			doEndCredits();
		break;
	}
}

static void doEndCredits()
{
	int i, r, g, b, remainingEntities;
	Target *t;
	SDL_Surface *surface;

	if (credits.creditLine == NULL)
	{
		initCredits();
	}

	remainingEntities = 0;

	for (i=0;i<credits.lineCount;i++)
	{
		credits.creditLine[i].y -= credits.speed;

		if (credits.creditLine[i].y < -64)
		{
			destroyTexture(credits.creditLine[i].textImage);

			credits.creditLine[i].textImage = NULL;
		}

		else
		{
			remainingEntities++;

			if (credits.creditLine[i].y <= SCREEN_HEIGHT)
			{
				if (credits.creditLine[i].textImage == NULL && credits.creditLine[i].text[0] != '#')
				{
					r = credits.creditLine[i].r;
					g = credits.creditLine[i].g;
					b = credits.creditLine[i].b;

					surface = generateTransparentTextSurface(credits.creditLine[i].text, game.font, r, g, b, TRUE);
					
					credits.creditLine[i].textImage = convertSurfaceToTexture(surface, TRUE);
				}
			}
		}
	}

	credits.startDelay--;

	if (credits.startDelay <= 0)
	{
		credits.startDelay = 0;

		if (player.inUse == TRUE)
		{
			remainingEntities++;

			self = &player;

			self->dirY += GRAVITY_SPEED * self->weight;

			if (self->dirY >= MAX_AIR_SPEED)
			{
				self->dirY = MAX_AIR_SPEED;
			}

			else if (self->dirY > 0 && self->dirY < 1)
			{
				self->dirY = 1;
			}

			self->creditsAction();

			addToGrid(self);
		}

		remainingEntities += doCreditsEntities();

		credits.nextEntityDelay--;

		if (credits.nextEntityDelay <= 0)
		{
			if (getNextEntity() == FALSE)
			{
				if (remainingEntities == 0)
				{
					credits.fading = TRUE;
				}
			}

			t = getTargetByName("CREDITS_TARGET");

			setMapStartX(t->x);

			centerMapOnEntity(NULL);

			credits.nextEntityDelay = 300;
		}
	}

	if (credits.fading == TRUE)
	{
		fadeCredits();

		credits.alpha += 3;

		if (credits.alpha == 255)
		{
			freeCredits();

			player.flags |= NO_DRAW;

			freeEntities();

			credits.line = 0;
			credits.lineCount = 0;
			credits.entityID = 0;
			credits.fading = FALSE;
			credits.alpha = 255;
			credits.startDelay = 0;
			credits.nextEntityDelay = 0;

			credits.status = 1;

			if (game.kills == 0)
			{
				credits.status = 0;

				titleScreen();
			}
		}
	}
}

static void doGameStats()
{
	if (credits.creditLine == NULL)
	{
		initGameStats();
	}

	credits.startDelay--;

	if (credits.startDelay <= 0)
	{
		if (credits.fading == TRUE)
		{
			freeCredits();

			player.flags |= NO_DRAW;

			freeEntities();

			credits.line = 0;
			credits.lineCount = 0;
			credits.entityID = 0;
			credits.fading = FALSE;
			credits.alpha = 255;
			credits.startDelay = 0;
			credits.nextEntityDelay = 0;

			credits.status = 2;
		}

		else
		{
			credits.line += credits.line == 0 ? 1 : 2;

			if (credits.line > credits.lineCount)
			{
				credits.fading = TRUE;

				credits.line = credits.lineCount;
			}

			credits.startDelay = credits.line == credits.lineCount ? 120 : 60;
		}
	}
}

static void doDefeatedBosses()
{
	int remainingEntities;

	if (credits.creditLine == NULL)
	{
		initDefeatedBosses();
	}

	remainingEntities = 0;

	credits.startDelay--;

	if (credits.startDelay <= 0)
	{
		credits.startDelay = 0;

		remainingEntities = doCreditsEntities();

		if (remainingEntities == 0)
		{
			if (getNextBoss() == FALSE)
			{
				credits.fading = TRUE;
			}

			else
			{
				setMapStartX(0);
				setMapStartY(0);
			}
		}

		if (credits.fading == TRUE)
		{
			fadeOutMusic(4000);

			fadeCredits();

			credits.alpha++;

			if (credits.alpha >= 255)
			{
				freeCredits();

				player.flags |= NO_DRAW;

				freeEntities();

				credits.line = 0;
				credits.lineCount = 0;
				credits.entityID = 0;
				credits.fading = FALSE;
				credits.alpha = 255;
				credits.startDelay = 0;
				credits.nextEntityDelay = 0;

				credits.status = 3;
			}
		}
	}
}

static void doChaos()
{
	Entity *e;
	Target *t;
	SDL_Surface *surface;

	if (credits.creditLine == NULL)
	{
		e = addEnemy("boss/chaos", 0, 0);

		t = getTargetByName("CHAOS_TARGET");

		e->x = t->x;
		e->y = t->y;

		e->active = FALSE;

		e->alpha = 0;

		e = addKeyItem("item/chaos_chain_base", 0, 0);

		t = getTargetByName("CHAOS_CHAIN_TARGET");

		e->x = t->x;
		e->y = t->y;

		e->alpha = 0;

		credits.creditLine = malloc(sizeof(CreditLine) * 2);

		if (credits.creditLine == NULL)
		{
			showErrorAndExit("Failed to allocate %d bytes for end credits...", sizeof(CreditLine) * 2);
		}

		if (hasPersistance("map26") == TRUE)
		{
			STRNCPY(credits.creditLine[0].text, _("Chaos has been subdued"), MAX_LINE_LENGTH);

			STRNCPY(credits.creditLine[1].text, _("For now..."), MAX_LINE_LENGTH);
		}

		else
		{
			STRNCPY(credits.creditLine[0].text, _("Chaos will soon be free"), MAX_LINE_LENGTH);

			STRNCPY(credits.creditLine[1].text, _("To terrorise the world once again"), MAX_LINE_LENGTH);
		}

		credits.line = -1;

		surface = generateTransparentTextSurface(credits.creditLine[0].text, game.largeFont, 220, 220, 220, TRUE);
		
		credits.creditLine[0].textImage = convertSurfaceToTexture(surface, TRUE);

		surface = generateTransparentTextSurface(credits.creditLine[1].text, game.largeFont, 220, 220, 220, TRUE);
		
		credits.creditLine[1].textImage = convertSurfaceToTexture(surface, TRUE);

		credits.creditLine[0].x = (SCREEN_WIDTH - credits.creditLine[0].textImage->w) / 2;

		credits.creditLine[1].x = (SCREEN_WIDTH - credits.creditLine[1].textImage->w) / 2;

		credits.creditLine[0].y = 80;

		credits.creditLine[1].y = credits.creditLine[0].y + 80;

		credits.creditLine[0].r = 255;

		credits.creditLine[1].r = 540;

		credits.startDelay = 180;

		credits.fadeSurface.x = 0;
		credits.fadeSurface.y = 0;
		credits.fadeSurface.w = SCREEN_WIDTH;
		credits.fadeSurface.h = MAX(credits.creditLine[0].textImage->h, credits.creditLine[1].textImage->h);
		
		credits.fadeSurface.r = 0;
		credits.fadeSurface.g = 0;
		credits.fadeSurface.b = 0;
	}

	doCreditsEntities();

	if (credits.line == -1)
	{
		credits.creditLine[0].r += credits.line;

		credits.creditLine[1].r += credits.line;

		if (credits.creditLine[0].r <= 0)
		{
			credits.creditLine[0].r = 0;
		}

		if (credits.creditLine[1].r <= 0)
		{
			credits.creditLine[1].r = 0;
		}

		if (credits.creditLine[1].r <= 0)
		{
			activateEntitiesWithObjectiveName("CHAOS", TRUE);

			credits.startDelay--;

			if (credits.startDelay <= 0)
			{
				credits.startDelay = 180;

				credits.line = 1;
				
				credits.fadeSurface.x = 0;
				credits.fadeSurface.y = 0;
				credits.fadeSurface.w = SCREEN_WIDTH;
				credits.fadeSurface.h = SCREEN_HEIGHT;
				
				credits.fadeSurface.r = 0;
				credits.fadeSurface.g = 0;
				credits.fadeSurface.b = 0;
			}
		}
	}

	else
	{
		credits.startDelay--;

		if (credits.startDelay <= 0)
		{
			credits.creditLine[0].r += credits.line;

			credits.creditLine[1].r += credits.line;

			if (credits.creditLine[0].r >= 255)
			{
				credits.creditLine[0].r = 255;

				credits.creditLine[1].r = 255;

				credits.startDelay--;

				if (credits.startDelay <= 0)
				{
					freeCredits();

					player.flags |= NO_DRAW;

					freeEntities();

					credits.line = 0;
					credits.lineCount = 0;
					credits.entityID = 0;
					credits.fading = FALSE;
					credits.alpha = 255;
					credits.startDelay = 0;
					credits.nextEntityDelay = 0;

					credits.status = 4;
				}
			}
		}
	}
}

static void doEdgarLogo()
{
	char copyright[MAX_VALUE_LENGTH];
	SDL_Surface *surface;

	if (credits.edgarLogo == NULL)
	{
		credits.creditLine = malloc(sizeof(CreditLine));

		if (credits.creditLine == NULL)
		{
			showErrorAndExit("Failed to allocate %d bytes for end credits...", sizeof(CreditLine));
		}

		snprintf(copyright, MAX_VALUE_LENGTH, _("Copyright Parallel Realities 2009 - %d"), YEAR);

		STRNCPY(credits.creditLine[0].text, copyright, sizeof(credits.creditLine[0].text));

		surface = generateTransparentTextSurface(credits.creditLine[0].text, game.font, 220, 220, 220, TRUE);
		
		credits.creditLine[0].textImage = convertSurfaceToTexture(surface, TRUE);

		credits.edgarLogo = loadImage("gfx/title_screen/logo.png");

		credits.prLogo = loadImage("gfx/title_screen/alien_device.png");

		credits.alpha = 255;

		credits.fadeSurface.x = 0;
		credits.fadeSurface.y = 0;
		credits.fadeSurface.w = SCREEN_WIDTH;
		credits.fadeSurface.h = SCREEN_HEIGHT;
		
		credits.fadeSurface.r = 0;
		credits.fadeSurface.g = 0;
		credits.fadeSurface.b = 0;

		credits.line = -2;

		credits.startDelay = 300;
	}

	credits.alpha += credits.line;

	if (credits.alpha <= 0)
	{
		credits.alpha = 0;

		credits.startDelay--;

		if (credits.startDelay <= 0)
		{
			credits.line = 2;

			credits.startDelay = 60;
		}
	}

	else if (credits.alpha >= 255)
	{
		credits.alpha = 255;

		freeCredits();

		player.flags |= NO_DRAW;

		freeEntities();

		credits.line = 0;
		credits.lineCount = 0;
		credits.entityID = 0;
		credits.fading = FALSE;
		credits.alpha = 255;
		credits.startDelay = 0;
		credits.nextEntityDelay = 0;

		credits.status = 0;

		titleScreen();
	}
}

void drawCredits()
{
	switch (credits.status)
	{
		case 1:
			drawGameStats();
		break;

		case 2:
			drawDefeatedBosses();
		break;

		case 3:
			drawChaos();
		break;

		case 4:
			drawEdgarLogo();
		break;

		default:
			drawEndCredits();
		break;
	}
}

static void drawEndCredits()
{
	int i;

	for (i=0;i<credits.lineCount;i++)
	{
		if (credits.creditLine[i].textImage != NULL)
		{
			drawImage(credits.creditLine[i].textImage, (SCREEN_WIDTH - credits.creditLine[i].textImage->w) / 2, credits.creditLine[i].y, FALSE, 255);
		}
	}

	if (credits.fading == TRUE)
	{
		drawBox(credits.fadeSurface.x, credits.fadeSurface.y, credits.fadeSurface.w, credits.fadeSurface.h, credits.fadeSurface.r, credits.fadeSurface.g, credits.fadeSurface.b, credits.alpha);
	}
}

static void drawGameStats()
{
	int i;

	if (credits.creditLine != NULL)
	{
		for (i=0;i<credits.line;i++)
		{
			drawImage(credits.creditLine[i].textImage, credits.creditLine[i].x, credits.creditLine[i].y, FALSE, 255);
		}
	}
}

static void drawDefeatedBosses()
{
	int i;

	if (credits.creditLine != NULL)
	{
		for (i=0;i<credits.lineCount;i++)
		{
			if (i != 0 && credits.startDelay > 0)
			{
				continue;
			}

			if (credits.creditLine[i].textImage != NULL)
			{
				drawImage(credits.creditLine[i].textImage, (SCREEN_WIDTH - credits.creditLine[i].textImage->w) / 2, credits.creditLine[i].y, FALSE, 255);
			}
		}
	}

	if (credits.fading == TRUE)
	{
		drawBox(credits.fadeSurface.x, credits.fadeSurface.y, credits.fadeSurface.w, credits.fadeSurface.h, credits.fadeSurface.r, credits.fadeSurface.g, credits.fadeSurface.b, credits.alpha);
	}
}

static void drawChaos()
{
	if (credits.creditLine != NULL)
	{
		drawImage(credits.creditLine[0].textImage, credits.creditLine[0].x, credits.creditLine[0].y, FALSE, 255);
		
		drawBox(0, credits.creditLine[0].y, credits.fadeSurface.w, credits.fadeSurface.h, credits.fadeSurface.r, credits.fadeSurface.g, credits.fadeSurface.b, credits.creditLine[0].r);

		if (credits.creditLine[1].r <= 255)
		{
			drawImage(credits.creditLine[1].textImage, credits.creditLine[1].x, credits.creditLine[1].y, FALSE, 255);

			drawBox(0, credits.creditLine[1].y, credits.fadeSurface.w, credits.fadeSurface.h, credits.fadeSurface.r, credits.fadeSurface.g, credits.fadeSurface.b, credits.creditLine[1].r);
		}
	}
}

static void drawEdgarLogo()
{
	int height;

	if (credits.edgarLogo != NULL)
	{
		height = credits.creditLine[0].textImage->h + 32;

		height += credits.edgarLogo->h + 32;

		height += credits.prLogo->h;

		height = (SCREEN_HEIGHT - height) / 2;

		drawImage(credits.edgarLogo, (SCREEN_WIDTH - credits.edgarLogo->w) / 2, height, FALSE, 255);

		height += credits.edgarLogo->h + 32;

		drawImage(credits.prLogo, (SCREEN_WIDTH - credits.prLogo->w) / 2, height, FALSE, 255);

		height += credits.prLogo->h + 32;

		drawImage(credits.creditLine[0].textImage, (SCREEN_WIDTH - credits.creditLine[0].textImage->w) / 2, height, FALSE, 255);

		drawBox(credits.fadeSurface.x, credits.fadeSurface.y, credits.fadeSurface.w, credits.fadeSurface.h, credits.fadeSurface.r, credits.fadeSurface.g, credits.fadeSurface.b, credits.alpha);
	}
}

static void initCredits()
{
	int lineNum, y;
	char *buffer, *token1, *token2, *savePtr1, *savePtr2;
	Target *t;

	buffer = (char *)loadFileFromPak("data/credits.dat");

	credits.lineCount = countTokens(buffer, "\n");

	credits.creditLine = malloc(credits.lineCount * sizeof(CreditLine));

	if (credits.creditLine == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for end credits...", credits.lineCount * sizeof(CreditLine));
	}

	lineNum = 0;

	token1 = strtok_r(buffer, "\n", &savePtr1);

	y = SCREEN_HEIGHT + 32;

	while (token1 != NULL)
	{
		token2 = strtok_r(token1, " ", &savePtr2);

		credits.creditLine[lineNum].r = atoi(token2);

		token2 = strtok_r(NULL, " ", &savePtr2);

		credits.creditLine[lineNum].g = atoi(token2);

		token2 = strtok_r(NULL, " ", &savePtr2);

		credits.creditLine[lineNum].b = atoi(token2);

		token2 = strtok_r(NULL, "\0", &savePtr2);

		STRNCPY(credits.creditLine[lineNum].text, token2, MAX_LINE_LENGTH);

		credits.creditLine[lineNum].y = y;

		credits.creditLine[lineNum].textImage = NULL;

		y += 32;

		lineNum++;

		token1 = strtok_r(NULL, "\n", &savePtr1);
	}

	credits.speed = y / 17500.0f;

	free(buffer);

	saveTemporaryData();

	freeInventory();

	freeLevelResources();

	loadMap("map_credits", TRUE);

	t = getTargetByName("CREDITS_TARGET");

	setMapStartX(t->x);

	credits.entityID = 0;

	player.flags |= NO_DRAW;

	credits.startDelay = 600;

	shuffleEnemies();
}

static void initGameStats()
{
	int i, x, y;
	SDL_Surface *surface;

	credits.lineCount = 15;

	credits.creditLine = malloc(credits.lineCount * sizeof(CreditLine));

	if (credits.creditLine == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for end credits...", credits.lineCount * sizeof(CreditLine));
	}

	STRNCPY(credits.creditLine[0].text, _("Statistics"), MAX_LINE_LENGTH);

	snprintf(credits.creditLine[1].text, MAX_LINE_LENGTH, "%s", _("Play Time"));
	snprintf(credits.creditLine[2].text, MAX_LINE_LENGTH, "%s", getPlayTimeAsString());

	snprintf(credits.creditLine[3].text, MAX_LINE_LENGTH, "%s", _("Kills"));
	snprintf(credits.creditLine[4].text, MAX_LINE_LENGTH, "%d", game.kills);

	snprintf(credits.creditLine[5].text, MAX_LINE_LENGTH, "%s", _("Arrows Fired"));
	snprintf(credits.creditLine[6].text, MAX_LINE_LENGTH, "%d", game.arrowsFired);

	snprintf(credits.creditLine[7].text, MAX_LINE_LENGTH, "%s", _("Distanced Travelled"));
	snprintf(credits.creditLine[8].text, MAX_LINE_LENGTH, "%ud", game.distanceTravelled / 45000);

	snprintf(credits.creditLine[7].text, MAX_LINE_LENGTH, "%s", _("Attacks Blocked"));
	snprintf(credits.creditLine[8].text, MAX_LINE_LENGTH, "%d", game.attacksBlocked);

	snprintf(credits.creditLine[9].text, MAX_LINE_LENGTH, "%s", _("Time Spent As A Slime"));
	snprintf(credits.creditLine[10].text, MAX_LINE_LENGTH, "%s", getSlimeTimeAsString());

	snprintf(credits.creditLine[11].text, MAX_LINE_LENGTH, "%s", _("Secrets Found"));
	snprintf(credits.creditLine[12].text, MAX_LINE_LENGTH, "%d / %d", game.secretsFound, TOTAL_SECRETS);

	snprintf(credits.creditLine[13].text, MAX_LINE_LENGTH, "%s", _("Continues"));
	snprintf(credits.creditLine[14].text, MAX_LINE_LENGTH, "%d", game.continues);

	y = 32;

	x = 0;

	for (i=0;i<credits.lineCount;i++)
	{
		surface = generateTransparentTextSurface(credits.creditLine[i].text, game.largeFont, 220, 220, 220, TRUE);
		
		credits.creditLine[i].textImage = convertSurfaceToTexture(surface, TRUE);

		if (credits.creditLine[i].textImage->w > x)
		{
			x = credits.creditLine[i].textImage->w;
		}
	}

	for (i=0;i<credits.lineCount;i++)
	{
		if (i == 0)
		{
			credits.creditLine[i].x = (SCREEN_WIDTH - credits.creditLine[i].textImage->w) / 2;
			credits.creditLine[i].y = y;

			y += credits.creditLine[i].textImage->h + 32;
		}

		else if (i % 2 == 1)
		{
			credits.creditLine[i].x = 64;
			credits.creditLine[i].y = y;
		}

		else
		{
			credits.creditLine[i].x = credits.creditLine[i - 1].x + x + 64;
			credits.creditLine[i].y = y;

			y += credits.creditLine[i].textImage->h + 16;
		}
	}

	credits.startDelay = 60;
}

static void initDefeatedBosses()
{
	SDL_Surface *surface;
	
	credits.lineCount = 3;

	credits.creditLine = malloc(credits.lineCount * sizeof(CreditLine));

	if (credits.creditLine == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for end credits...", credits.lineCount * sizeof(CreditLine));
	}

	STRNCPY(credits.creditLine[0].text, _("Defeated Bosses"), MAX_LINE_LENGTH);

	credits.creditLine[0].y = 64;

	surface = generateTransparentTextSurface(credits.creditLine[0].text, game.largeFont, 220, 220, 220, TRUE);
	
	credits.creditLine[0].textImage = convertSurfaceToTexture(surface, TRUE);

	STRNCPY(credits.creditLine[1].text, "", MAX_LINE_LENGTH);

	credits.creditLine[1].y = 350;

	credits.creditLine[1].textImage = NULL;

	STRNCPY(credits.creditLine[2].text, "", MAX_LINE_LENGTH);

	credits.creditLine[2].y = 400;

	credits.creditLine[2].textImage = NULL;

	credits.startDelay = 60;

	player.flags |= NO_DRAW;
}

int countTokens(char *line, char *delim)
{
	char *temp, *savePtr, *token;
	int i;

	temp = malloc(strlen(line) + 1);

	if (temp == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for tokens...", (int)strlen(line) + 1);
	}

	STRNCPY(temp, line, strlen(line) + 1);

	token = strtok_r(temp, delim, &savePtr);

	i = 0;

	while (token != NULL)
	{
		i++;

		token = strtok_r(NULL, delim, &savePtr);
	}

	free(temp);

	return i;
}

void freeCredits()
{
	int i;

	if (credits.creditLine != NULL)
	{
		for (i=0;i<credits.lineCount;i++)
		{
			if (credits.creditLine[i].textImage != NULL)
			{
				destroyTexture(credits.creditLine[i].textImage);

				credits.creditLine[i].textImage = NULL;
			}
		}

		free(credits.creditLine);

		credits.creditLine = NULL;
	}

	if (credits.prLogo != NULL)
	{
		destroyTexture(credits.prLogo);

		credits.prLogo = NULL;
	}

	if (credits.edgarLogo != NULL)
	{
		destroyTexture(credits.edgarLogo);

		credits.edgarLogo = NULL;
	}
	
	credits.fadeSurface.w = 0;
}

static Entity *loadCreditsEntity(char *name)
{
	Entity *e;
	Target *t;

	t = getTargetByName("CREDITS_TARGET");

	if (strstr(name, "/edgar") != NULL)
	{
		e = loadPlayer(0, t->y, name);
	}

	else
	{
		e = addEnemy(name, 0, t->y);
	}

	e->face = RIGHT;

	e->startX = e->x;
	e->startY = e->y;

	return e;
}

static int getNextEntity()
{
	if (credits.entityID >= enemiesLength)
	{
		return FALSE;
	}

	loadCreditsEntity(enemies[credits.entityID]);

	credits.entityID++;

	return TRUE;
}

static Entity *loadCreditsBoss(char *name)
{
	Entity *e;
	SDL_Surface *surface;

	e = addEnemy(name, SCREEN_WIDTH, 0);

	e->y = (SCREEN_HEIGHT - e->h) / 2;

	e->flags &= ~NO_DRAW;

	e->flags |= FLY;

	e->targetX = (SCREEN_WIDTH - e->w) / 2;

	e->thinkTime = 120;

	credits.line = hasPersistance(getBossMap(e->name)) == TRUE && bossExists(getBossTrigger(e->name)) == FALSE ? TRUE : FALSE;

	STRNCPY(credits.creditLine[1].text, _(getBossName(e->name)), MAX_LINE_LENGTH);

	STRNCPY(credits.creditLine[2].text, credits.line == FALSE ? _("Undefeated") : _("Defeated"), MAX_LINE_LENGTH);

	if (credits.creditLine[1].textImage != NULL)
	{
		destroyTexture(credits.creditLine[1].textImage);

		credits.creditLine[1].textImage = NULL;
	}

	if (credits.creditLine[2].textImage != NULL)
	{
		destroyTexture(credits.creditLine[2].textImage);

		credits.creditLine[2].textImage = NULL;
	}

	surface = generateTransparentTextSurface(credits.creditLine[1].text, game.largeFont, 220, 220, 220, TRUE);
	
	credits.creditLine[1].textImage = convertSurfaceToTexture(surface, TRUE);

	return e;
}

static int getNextBoss()
{
	if (credits.entityID >= bossesLength)
	{
		return FALSE;
	}

	loadCreditsBoss(bosses[credits.entityID]);

	credits.entityID++;

	return TRUE;
}

void fadeCredits()
{
	if (credits.fadeSurface.w != SCREEN_WIDTH)
	{
		credits.fading = TRUE;

		credits.alpha = 0;

		credits.fadeSurface.x = 0;
		credits.fadeSurface.y = 0;
		credits.fadeSurface.w = SCREEN_WIDTH;
		credits.fadeSurface.h = SCREEN_HEIGHT;
		
		credits.fadeSurface.r = 0;
		credits.fadeSurface.g = 0;
		credits.fadeSurface.b = 0;
	}
}

void bossMoveToMiddle()
{
	SDL_Surface *surface;
	
	self->x -= 20;

	if (self->x <= self->targetX)
	{
		self->x = self->targetX;

		if (strlen(credits.creditLine[2].text) != 0 && credits.creditLine[2].textImage == NULL)
		{
			if (credits.line == TRUE)
			{
				surface = generateTransparentTextSurface(credits.creditLine[2].text, game.largeFont, 0, 220, 0, TRUE);
				
				credits.creditLine[2].textImage = convertSurfaceToTexture(surface, TRUE);
			}

			else
			{
				surface = generateTransparentTextSurface(credits.creditLine[2].text, game.largeFont, 220, 0, 0, TRUE);
				
				credits.creditLine[2].textImage = convertSurfaceToTexture(surface, TRUE);
			}
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->creditsAction = &bossMoveOffScreen;
		}
	}
}

static void bossMoveOffScreen()
{
	self->x -= 20;

	if (self->x <= -self->w)
	{
		self->inUse = FALSE;
	}
}

static char *getBossName(char *name)
{
	int i;

	for (i=0;i<bossesLength*4;i+=4)
	{
		if (strcmpignorecase(name, bossNames[i]) == 0)
		{
			return bossNames[i + 1];
		}
	}

	showErrorAndExit("Could not find name for boss %s", name);

	return NULL;
}

static char *getBossMap(char *name)
{
	int i;

	for (i=0;i<bossesLength*4;i+=4)
	{
		if (strcmpignorecase(name, bossNames[i]) == 0)
		{
			return bossNames[i + 2];
		}
	}

	showErrorAndExit("Could not find map for boss %s", name);

	return NULL;
}

static char *getBossTrigger(char *name)
{
	int i;

	for (i=0;i<bossesLength*4;i+=4)
	{
		if (strcmpignorecase(name, bossNames[i]) == 0)
		{
			return bossNames[i + 3];
		}
	}

	showErrorAndExit("Could not find trigger for boss %s", name);

	return NULL;
}

static void shuffleEnemies()
{
	char *s;
	int i, j;

	/* Skip Edgar and the Ghost. Leave the Master Tortoise last */

	for (i=2;i<enemiesLength-1;i++)
	{
		s = enemies[i];

		j = 2 + prand() % (enemiesLength - 3);

		enemies[i] = enemies[j];

		enemies[j] = s;
	}
}

static int doCreditsEntities()
{
	int remainingEntities;
	EntityList *el, *entities;

	entities = getEntities();

	remainingEntities = 0;

	for (el=entities->next;el!=NULL;el=el->next)
	{
		self = el->entity;

		if (self->inUse == TRUE)
		{
			remainingEntities++;

			self->takeDamage = NULL;

			if (!(self->flags & TELEPORTING))
			{
				if (!(self->flags & (FLY|GRABBED)))
				{
					switch (self->environment)
					{
						case WATER:
						case SLIME:
							self->dirY += GRAVITY_SPEED * 0.25 * self->weight;

							if (self->flags & FLOATS)
							{
								if (self->dirX != 0)
								{
									self->endY++;

									self->dirY = cos(DEG_TO_RAD(self->endY)) / 20;
								}
							}

							if (self->dirY >= MAX_WATER_SPEED)
							{
								self->dirY = MAX_WATER_SPEED;
							}
						break;

						default:
							self->dirY += GRAVITY_SPEED * self->weight;

							if (self->dirY >= MAX_AIR_SPEED)
							{
								self->dirY = MAX_AIR_SPEED;
							}

							else if (self->dirY > 0 && self->dirY < 1)
							{
								self->dirY = 1;
							}
						break;
					}
				}

				if (self->creditsAction == NULL)
				{
					showErrorAndExit("%s has no Credits Action defined", self->name);
				}

				self->creditsAction();

				addToGrid(self);

				addToDrawLayer(self, self->layer);
			}

			else
			{
				doTeleport();
			}
		}
	}

	return remainingEntities;
}
