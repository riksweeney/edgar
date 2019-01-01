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

#include "../headers.h"

#include "../entity.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "animation.h"
#include "decoration.h"
#include "graphics.h"

static EntityList *decoration;

extern Entity *self, player;
extern Game game;

static void move(void);
static void entityWait(void);
static void finish(void);
static void timeout(void);
static int drawPixel(void);

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

	showErrorAndExit("Could not find decoration %s", name);

	return NULL;
}

void freeDecorations()
{
	EntityList *p, *q;

	if (decoration != NULL)
	{
		for (p=decoration->next;p!=NULL;p=q)
		{
			free(p->entity);

			q = p->next;

			free(p);
		}

		free(decoration);
	}

	decoration = malloc(sizeof(EntityList));

	if (decoration == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Entity List", (int)sizeof(EntityList));
	}

	decoration->next = NULL;
}

Entity *getFreeDecoration()
{
	Entity *e;

	e = malloc(sizeof(Entity));

	if (e == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for a Decoration", (int)sizeof(Entity));
	}

	memset(e, 0, sizeof(Entity));

	e->inUse = TRUE;

	e->active = TRUE;

	e->frameSpeed = 1;

	e->alpha = 255;

	addEntityToList(decoration, e);

	return e;
}

void doDecorations()
{
	EntityList *el;

	/* Loop through the Decorations and perform their action */

	for (el=decoration->next;el!=NULL;el=el->next)
	{
		self = el->entity;

		if (self->inUse == TRUE)
		{
			self->action();
		}
	}
}

void drawDecorations()
{
	int removeCount;
	EntityList *el, *prev, *el2;

	for (el=decoration->next;el!=NULL;el=el->next)
	{
		self = el->entity;

		if (self->inUse == TRUE)
		{
			self->draw();
		}
	}

	if (game.frames % 300 == 0)
	{
		removeCount = 0;

		prev = decoration;

		for (el=decoration->next;el!=NULL;el=el2)
		{
			el2 = el->next;

			if (el->entity->inUse == FALSE)
			{
				prev->next = el2;

				removeCount++;

				free(el->entity);

				el->entity = NULL;

				free(el);

				el = NULL;
			}

			else
			{
				prev = prev->next;
			}
		}

		#if DEV == 1
		if (removeCount != 0)
		{
			printf("Removed %d decorations taking up %d bytes\n", removeCount, (int)sizeof(Entity) * removeCount);
		}
		#endif
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

		setEntityAnimationByID(e, prand() % 5);
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

	setEntityAnimationByID(e, prand() % 5);

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

	e->thinkTime = 300;

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

	if (self->thinkTime <= 0)
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
		showErrorAndExit("Decoration could not find Entity %s", entityName);
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

static int drawPixel()
{
	putPixelToMap(self->x, self->y, self->health, self->maxHealth, self->mental);

	return TRUE;
}
