/*
Copyright (C) 2009-2011 Parallel Realities

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

#include "headers.h"

#include "system/pak.h"
#include "system/error.h"
#include "credits.h"
#include "inventory.h"
#include "system/properties.h"
#include "system/resources.h"
#include "graphics/animation.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "enemy/enemies.h"
#include "player.h"
#include "map.h"
#include "entity.h"
#include "collisions.h"
#include "item/item.h"
#include "system/load_save.h"
#include "system/random.h"
#include "system/load_save.h"

extern Game game;
extern Entity *self, entity[MAX_ENTITIES], player;

static Credits credits;
static char *enemies[] = {
			"edgar/edgar",
			"enemy/chicken",
			"enemy/grub",
			"enemy/wasp",
			"enemy/red_grub",
			"enemy/jumping_slime",
			"enemy/purple_jumping_slime",
			"enemy/red_jumping_slime",
			"enemy/scorpion",
			"enemy/snail",
			"enemy/purple_snail",
			"enemy/red_bat",
			"enemy/gazer",
			"enemy/sludge",
			"enemy/red_sludge",
			"enemy/summoner",
			"enemy/centurion",
			"enemy/red_centurion",
			"boss/centurion_boss",
			"enemy/tortoise",
			"enemy/ice_tortoise",
			"enemy/fire_tortoise",
			"enemy/ceiling_crawler",
			"enemy/large_spider",
			"enemy/large_red_spider",
			"enemy/dragon_fly"
};
static int length = sizeof(enemies) / sizeof(char *);

static void initCredits(void);
static Entity *loadCreditsEntity(char *);
static int getNextEntity(void);

void doCredits()
{
	int i, r, g, b, remainingEntities;
	
	if (credits.creditLine == NULL)
	{
		initCredits();
	}
	
	credits.logoY -= 0.25;
	
	if (credits.logoY < -SCREEN_HEIGHT)
	{
		SDL_FreeSurface(credits.logo);
		
		credits.logo = NULL;
	}
	
	remainingEntities = 0;
	
	for (i=0;i<credits.lineCount;i++)
	{
		credits.creditLine[i].y -= 0.25;
		
		if (credits.creditLine[i].y < -SCREEN_HEIGHT)
		{
			SDL_FreeSurface(credits.creditLine[i].textImage);
			
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
					
					credits.creditLine[i].textImage = generateTransparentTextSurface(credits.creditLine[i].text, game.font, r, g, b, TRUE);
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
		
		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == TRUE)
			{
				if (!(self->flags & TELEPORTING))
				{
					remainingEntities++;
					
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
		
		credits.nextEntityDelay--;
		
		if (credits.nextEntityDelay <= 0)
		{
			if (getNextEntity() == FALSE && remainingEntities == 0)
			{
				credits.fading = TRUE;
			}
			
			setMapStartX(TILE_SIZE * 4);
			
			centerMapOnEntity(NULL);
			
			credits.nextEntityDelay = 300;
		}
	}
	
	if (credits.fading == TRUE)
	{
		fadeCredits();
		
		credits.alpha++;
		
		if (credits.alpha == 255)
		{
			freeCredits();
			
			printf("Defeated King Grub: %d\n", bossExists("boss/grub_boss"));
			printf("Defeated Golem: %d\n", bossExists("boss/golem_boss"));
			printf("Defeated Queen Fly: %d\n", bossExists("boss/fly_boss"));
			printf("Defeated Swamp Guardian: %d\n", bossExists("boss/snake_grub"));
			printf("Defeated Blob: %d\n", bossExists("boss/blob_boss_2"));
			printf("Defeated Mataeus: %d\n", bossExists("boss/mataeus"));
			printf("Defeated Phoenix: %d\n", bossExists("boss/phoenix"));
			printf("Defeated Awesome Foursome: %d\n", bossExists("boss/awesome_boss_1"));
			printf("Defeated Watchdog: %d\n", bossExists("boss/armour_boss"));
			printf("Defeated Borer: %d\n", bossExists("boss/borer_boss"));
			printf("Defeated Evil Edgar: %d\n", bossExists("boss/evil_edgar"));
			printf("Defeated Sewer Dweller: %d\n", bossExists("boss/sewer_boss"));
			printf("Defeated Salamander: %d\n", bossExists("boss/cave_boss"));
			printf("Defeated Grimlore: %d\n", bossExists("boss/grimlore"));
			
			exit(0);
		}
	}
}

void drawCredits()
{
	int i;
	SDL_Rect rect;
	
	rect.x = 0;
	rect.y = 0;
	rect.w = SCREEN_WIDTH;
	rect.h = SCREEN_HEIGHT - 160;
	
	/*SDL_SetClipRect(game.screen, &rect);*/
	
	if (credits.logo != NULL)
	{
		drawImage(credits.logo, (SCREEN_WIDTH - credits.logo->w) / 2, credits.logoY, FALSE, 255);
	}	
	
	for (i=0;i<credits.lineCount;i++)
	{
		if (credits.creditLine[i].textImage != NULL)
		{
			drawImage(credits.creditLine[i].textImage, (SCREEN_WIDTH - credits.creditLine[i].textImage->w) / 2, credits.creditLine[i].y, FALSE, 255);
		}
	}
	
	/*SDL_SetClipRect(game.screen, NULL);*/
	
	if (credits.fading == TRUE)
	{
		drawImage(credits.fadeSurface, 0, 0, FALSE, credits.alpha);
	}
}

static void initCredits()
{
	int lineNum, y;
	char *buffer, *token1, *token2, *savePtr1, *savePtr2;
	
	credits.logo = loadImage("gfx/title_screen/logo.png");
	
	buffer = (char *)loadFileFromPak("data/credits");
	
	credits.lineCount = countTokens(buffer, "\n");
	
	credits.creditLine = malloc(credits.lineCount * sizeof(CreditLine));
	
	if (credits.creditLine == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for end credits...", credits.lineCount * sizeof(CreditLine));
	}
	
	lineNum = 0;
	
	token1 = strtok_r(buffer, "\n", &savePtr1);
	
	y = SCREEN_HEIGHT;
	
	credits.logoY = y;
	
	y += credits.logo->h + 32;
	
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
	
	free(buffer);
	
	saveTemporaryData();
	
	freeInventory();

	freeLevelResources();
	
	loadMap("map_credits", TRUE);
	
	setMapStartX(TILE_SIZE * 4);
	
	credits.entityID = 0;
	
	player.inUse = FALSE;
	
	credits.startDelay = 300;
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
				SDL_FreeSurface(credits.creditLine[i].textImage);
				
				credits.creditLine[i].textImage = NULL;
			}
		}
		
		free(credits.creditLine);
	}
	
	if (credits.logo != NULL)
	{
		SDL_FreeSurface(credits.logo);
		
		credits.logo = NULL;
	}
	
	if (credits.fadeSurface != NULL)
	{
		SDL_FreeSurface(credits.fadeSurface);
		
		credits.fadeSurface = NULL;
	}
}

static Entity *loadCreditsEntity(char *name)
{
	Entity *e;
	
	if (strstr(name, "/edgar") != NULL)
	{
		e = loadPlayer(0, 0, name);
		
		addPermanentItem("weapon/fire_shield", 0, 300);
		
		addPermanentItem("weapon/basic_sword", 0, 300);
	}
	
	else
	{
		e = addEnemy(name, 0, 0);
	}
	
	e->x = 0;
	
	e->y = 250;
	
	e->face = RIGHT;
	
	e->startX = e->x;
	e->startY = e->y;
	
	return e;
}

static int getNextEntity()
{
	printf("Getting %d of %d\n", credits.entityID, length);
	
	if (credits.entityID >= length)
	{
		return FALSE;
	}
	
	loadCreditsEntity(enemies[credits.entityID]);
	
	credits.entityID++;
	
	return TRUE;
}

void fadeCredits()
{
	if (credits.fadeSurface == NULL)
	{
		credits.fading = TRUE;
		
		credits.alpha = 0;
		
		credits.fadeSurface = createSurface(game.screen->w, game.screen->h);

		drawBox(credits.fadeSurface, 0, 0, game.screen->w, game.screen->h, 0, 0, 0);
	}
}
