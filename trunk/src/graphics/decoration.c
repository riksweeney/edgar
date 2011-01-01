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

#include "../headers.h"

#include "decoration.h"
#include "font.h"
#include "animation.h"
#include "graphics.h"
#include "sprites.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../entity.h"
#include "../map.h"
#include "../system/error.h"

static Entity decoration[MAX_DECORATIONS];
static int decorationIndex = 0;

extern Entity *self, player;
extern Game game;

static void move(void);
static void entityWait(void);
static void finish(void);
static void timeout(void);
static int drawPixel(void);
static void riseUp(void);
static int drawScore(void);

static Constructor decorations[] = {
{"decoration/chimney_smoke", &addSmoke},
{"decoration/multiple_sparkles", &addMultipleSparkles}
};

static int length = sizeof(decorations) / sizeof(Constructor);

Entity *addDecoration(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(decorations[i].name, name) == 0)
		{
			return decorations[i].construct(x, y, name);
		}
	}

	showErrorAndExit("Could not find decoration %s\n", name);

	return NULL;
}

void freeDecorations()
{
	/* Clear the list */

	memset(decoration, 0, sizeof(Entity) * MAX_DECORATIONS);
}

Entity *getFreeDecoration()
{
	int i, count;

	count = 0;

	/* Loop through all the Decorations and find a free slot */

	for (i=decorationIndex;;i++)
	{
		if (i >= MAX_DECORATIONS)
		{
			i = 0;
		}

		if (decoration[i].inUse == FALSE)
		{
			memset(&decoration[i], 0, sizeof(Entity));

			decoration[i].inUse = TRUE;

			decoration[i].active = TRUE;

			decoration[i].frameSpeed = 1;

			decoration[i].alpha = 255;

			decorationIndex = i + 1;

			return &decoration[i];
		}

		count++;

		if (count >= MAX_DECORATIONS)
		{
			break;
		}
	}

	/* Return NULL if you couldn't any free slots */

	return NULL;
}

void doDecorations()
{
	int i;

	/* Loop through the Decorations and perform their action */

	for (i=0;i<MAX_DECORATIONS;i++)
	{
		self = &decoration[i];

		if (self->inUse == TRUE)
		{
			self->action();
		}
	}
}

void drawDecorations()
{
	int i;

	/* Loop through the Decorations and perform their action */

	for (i=0;i<MAX_DECORATIONS;i++)
	{
		self = &decoration[i];

		if (self->inUse == TRUE)
		{
			self->draw();
		}
	}
}

void addStarExplosion(int x, int y)
{
	int i;
	Entity *e;

	for (i=0;i<40;i++)
	{
		e = getFreeDecoration();

		if (e == NULL)
		{
			return;
		}

		loadProperties("decoration/star", e);

		e->dirX = prand() % 20;
		e->dirY = prand() % 20;

		if (prand() % 2 == 0)
		{
			e->dirX *= -1;
		}

		if (prand() % 2 == 0)
		{
			e->dirY *= -1;
		}

		e->dirX /= 10;
		e->dirY /= 10;

		e->thinkTime = 20 + (prand() % 30);

		e->x = x;
		e->y = y;

		e->action = &move;
		e->draw = &drawLoopingAnimationToMap;
	}
}

void addParticleExplosion(int x, int y)
{
	int i;
	Entity *e;

	for (i=0;i<40;i++)
	{
		e = getFreeDecoration();

		if (e == NULL)
		{
			return;
		}

		loadProperties("decoration/particle", e);

		e->dirX = prand() % 20;
		e->dirY = prand() % 20;

		if (prand() % 2 == 0)
		{
			e->dirX *= -1;
		}

		if (prand() % 2 == 0)
		{
			e->dirY *= -1;
		}

		e->dirX /= 10;
		e->dirY /= 10;

		e->thinkTime = 20 + (prand() % 30);

		e->x = x;
		e->y = y;

		e->action = &move;
		e->draw = &drawLoopingAnimationToMap;

		setEntityAnimation(e, prand() % 5);
	}
}

Entity *addParticle(int x, int y)
{
	Entity *e;

	e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	loadProperties("decoration/particle", e);

	e->thinkTime = 20 + (prand() % 30);

	e->x = x;
	e->y = y;

	e->action = &move;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, prand() % 5);

	return e;
}

Entity *addMultipleSparkles(int x, int y, char *name)
{
	int i, xx, yy;

	for (i=0;i<5;i++)
	{
		xx = x + (prand() % 30) * (prand() % 2 == 0 ? -1 : 1);
		yy = y + (prand() % 30) * (prand() % 2 == 0 ? -1 : 1);

		addSparkle(xx, yy);
	}

	return NULL;
}

void addSparkle(int x, int y)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return;
	}

	loadProperties("decoration/sparkle", e);

	e->thinkTime = 15;

	e->x = x;
	e->y = y;

	e->action = &entityWait;
	e->draw = &drawLoopingAnimationToMap;
	e->animationCallback = &finish;
}

Entity *addTrail(int x, int y, char *name, int thinkTime)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	loadProperties(name, e);

	e->thinkTime = thinkTime;

	e->x = x;
	e->y = y;

	e->action = &timeout;
	e->draw = &drawLoopingAnimationToMap;

	return e;
}

Entity *addSmoke(int x, int y, char *name)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->dirX = 0;

	e->dirY = -e->speed;

	e->frameSpeed *= 10;

	e->frameSpeed /= 1 + prand() % 10;

	e->action = &move;
	e->draw = &drawLoopingAnimationToMap;
	e->animationCallback = &finish;

	return e;
}

static void finish()
{
	self->inUse = FALSE;
}

static void timeout()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		finish();
	}
}

static void entityWait()
{
	self->x += self->dirX;
	self->y += self->dirY;
}

static void move()
{
	self->x += self->dirX;
	self->y += self->dirY;

	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->inUse = FALSE;
	}
}

void addBlood(int x, int y)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return;
	}

	loadProperties("decoration/blood", e);

	e->thinkTime = 60;

	e->x = x;
	e->y = y;

	e->dirY = 0.1;

	e->action = &move;
	e->draw = &drawLoopingAnimationToMap;
}

Entity *addBasicDecoration(int x, int y, char *name)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &move;
	e->draw = &drawLoopingAnimationToMap;

	return e;
}

void addDecorationFromScript(char *line)
{
	char decorationName[MAX_VALUE_LENGTH], entityName[MAX_VALUE_LENGTH];
	Entity *e;

	sscanf(line, "%s \"%[^\"]\"", decorationName, entityName);

	if (strcmpignorecase(entityName, "Edgar") == 0)
	{
		e = &player;
	}

	else
	{
		e = getEntityByObjectiveName(entityName);
	}

	if (e == NULL)
	{
		showErrorAndExit("Decoration could not find Entity %s\n", entityName);
	}

	addDecoration(decorationName, e->x + e->w / 2, e->y + e->h / 2);
}

Entity *addPixelDecoration(int x, int y)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	e->x = x;
	e->y = y;

	e->action = &move;
	e->draw = &drawPixel;

	return e;
}

Entity *addDamageScore(int damage, Entity *parentEntity)
{
	char damageText[5], absDamageText[5];
	int index;
	SDL_Surface *image;
	Sprite *sprite;
	Entity *e;

	return NULL;

	e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	snprintf(damageText, 5, "%d", damage);

	snprintf(absDamageText, 5, "%d", abs(damage));

	index = getSpriteIndexByName(damageText);

	if (index == -1)
	{
		if (damage < 0)
		{
			image = generateTransparentTextSurface(absDamageText, game.font, 0, 220, 0, FALSE);
		}

		else
		{
			image = generateTransparentTextSurface(absDamageText, game.font, 220, 220, 220, FALSE);
		}

		index = createSpriteFromSurface(damageText, image);
	}

	else
	{
		sprite = getSprite(index);

		image = sprite->image;
	}

	e->health = index;

	e->thinkTime = 60;

	e->action = &riseUp;
	e->draw = &drawScore;

	e->face = RIGHT;

	e->x = parentEntity->x + parentEntity->w / 2 - image->w / 2;
	e->y = parentEntity->y + parentEntity->h / 2 - image->h / 2;

	e->x += prand() % 10 * (prand() % 2 == 0 ? -1 : 1);
	e->y += prand() % 10 * (prand() % 2 == 0 ? -1 : 1);

	return e;
}

static void riseUp()
{
	self->y--;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static int drawScore()
{
	int x, y;
	int startX, startY;
	Sprite *sprite;

	startX = getMapStartX();
	startY = getMapStartY();

	x = self->x - startX;
	y = self->y - startY;

	sprite = getSprite(self->health);

	drawImage(sprite->image, x, y, FALSE, 255);

	return TRUE;
}

static int drawPixel()
{
	putPixelToMap(self->x, self->y, self->health, self->maxHealth, self->mental);

	return TRUE;
}
