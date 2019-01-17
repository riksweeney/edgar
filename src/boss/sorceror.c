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

#include "../audio/audio.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"
#include "sorceror.h"

extern Entity *self, player;

static void init(void);
static void stand(void);
static void stand2(void);
static void stand3(void);
static void dungeonTeleportInit(void);
static void offscreenTeleportInit(void);
static void dungeonTeleportWait(void);
static void teleportPlayerToDungeon(Entity *);
static void spellMove(void);
static void teleportOffscreenWait(void);
static void teleportPlayerOffscreen(Entity *);
static void arrowTakeDamage(Entity *, int);
static void disintegrationInit(void);
static void disintegrationWait(void);
static void disintegrationAttack(void);
static void disintegrationWait(void);
static void disintegration(void);
static void disintegrationTouch(Entity *);

Entity *addSorceror(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case -5:
			self->action = &stand;
		break;

		case -4:
			self->action = &stand2;

			self->touch = &entityTouch;

			self->takeDamage = &arrowTakeDamage;
		break;

		default:
			self->action = &stand3;
		break;
	}
}

static void stand()
{
	if (self->mental == -4)
	{
		setEntityAnimation(self, "ATTACK_1");

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &dungeonTeleportInit;
	}

	checkToMap(self);
}

static void stand2()
{
	if (self->mental == -3)
	{
		setEntityAnimation(self, "ATTACK_1");

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &offscreenTeleportInit;
	}

	checkToMap(self);
}

static void stand3()
{
	if (self->mental == -2)
	{
		self->action = &disintegrationInit;
	}

	checkToMap(self);
}

static void dungeonTeleportInit()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror Spell");
	}

	loadProperties("boss/sorceror_teleport_spell", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + e->offsetX;
	e->y = self->y + e->offsetY;

	e->action = &spellMove;

	calculatePath(e->x, e->y, player.x + player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);

	e->dirX *= e->speed;
	e->dirY *= e->speed;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &teleportPlayerToDungeon;

	self->thinkTime = 60;

	self->action = &dungeonTeleportWait;
}

static void dungeonTeleportWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");
	}

	checkToMap(self);
}

static void spellMove()
{
	int i;
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	else
	{
		for (i=0;i<2;i++)
		{
			e = addBasicDecoration(self->x, self->y, "decoration/particle");

			if (e != NULL)
			{
				e->x += prand() % self->w;
				e->y += prand() % self->h;

				e->thinkTime = 5 + prand() % 30;

				setEntityAnimationByID(e, prand() % 5);
			}
		}
	}
}

static void teleportPlayerToDungeon(Entity *other)
{
	player.flags |= NO_DRAW;

	self->inUse = FALSE;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/teleport", EDGAR_CHANNEL, self->x, self->y, 0);
}

static void offscreenTeleportInit()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror Spell");
	}

	loadProperties("boss/sorceror_teleport_spell", e);

	setEntityAnimation(e, "STAND");

	e->face = self->face;

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - e->offsetX;
	}

	else
	{
		e->x = self->x + e->offsetX;
	}

	e->y = self->y + e->offsetY;

	e->action = &spellMove;

	calculatePath(e->x, e->y, player.x + player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);

	e->dirX *= e->speed;
	e->dirY *= e->speed;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &teleportPlayerOffscreen;

	self->thinkTime = 60;

	self->action = &teleportOffscreenWait;
}

static void teleportOffscreenWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		setEntityAnimation(self, "STAND");

		if (self->mental == -2)
		{
			self->action = &disintegrationInit;
		}
	}
}

static void disintegrationInit()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Disintegration Spell");
	}

	setEntityAnimation(self, "ATTACK_2");

	loadProperties("boss/sorceror_disintegration_spell", e);

	setEntityAnimation(e, "STAND");

	e->face = self->face;

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - e->offsetX;
	}

	else
	{
		e->x = self->x + e->offsetX;
	}

	e->y = self->y + e->offsetY;

	e->action = &disintegration;

	e->thinkTime = 120;

	e->startX = e->x;
	e->startY = e->y;

	e->head = self;

	e->endX = player.x + player.w / 2;
	e->endY = player.y + player.h / 2;

	e->dirX = self->face == LEFT ? -1 : 1;

	e->draw = &drawLoopingAnimationToMap;

	self->action = &disintegrationWait;
}

static void disintegrationWait()
{
	checkToMap(self);
}

static void disintegration()
{
	Entity *e = addPixelDecoration(self->x, self->y);

	if (e != NULL)
	{
		e->x = self->x + (prand() % 32) * (prand() % 2 == 0 ? -1 : 1) + self->w / 2;
		e->y = self->y + (prand() % 32) * (prand() % 2 == 0 ? -1 : 1) + self->h / 2;

		e->startX = e->x;
		e->startY = e->y;

		e->endX = self->x;
		e->endY = self->y;

		e->thinkTime = 15;

		e->health = 230;

		e->maxHealth = 230;

		e->mental = 230;

		calculatePath(e->startX, e->startY, e->endX, e->endY, &e->dirX, &e->dirY);
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/boss/sorceror/electrocute", BOSS_CHANNEL, self->x, self->y, 0);

		self->x = self->endX;
		self->y = self->endY;

		self->thinkTime = 300;

		self->touch = &disintegrationTouch;

		self->action = &disintegrationAttack;

		self->draw = &drawDisintegrationSpell;

		self->layer = BACKGROUND_LAYER;

		self->flags &= ~NO_DRAW;
	}
}

static void disintegrationTouch(Entity *other)
{
	Entity *temp;

	if (!(other->flags & BLOCKING))
	{
		setEntityAnimation(&player, "CUSTOM_1");

		self->mental = 1;
	}

	else if (self->thinkTime > 90)
	{
		temp = self;

		self = other;

		other->takeDamage(temp, 1);

		self = temp;

		self->touch = NULL;

		self->thinkTime = 90;
	}
}

int drawDisintegrationSpell()
{
	Colour colour1, colour2, colour3;

	colour1.r = 231;
	colour1.g = 231;
	colour1.b = 231;
	colour1.a = 255;
	
	colour2.r = 57;
	colour2.g = 57;
	colour2.b = 224;
	colour2.a = 255;
	
	colour3.r = 41;
	colour3.g = 41;
	colour3.b = 160;
	colour3.a = 255;

	drawDisintegrationLine(self->startX, self->startY, self->endX, self->endY, colour1, colour2, colour3);

	return TRUE;
}

static void disintegrationAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->mental = -1;

		self->inUse = FALSE;

		if (self->mental == 1)
		{
			player.flags |= NO_DRAW;

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			playSoundToMap("sound/common/teleport", EDGAR_CHANNEL, self->x, self->y, 0);
		}
	}
}

static void teleportPlayerOffscreen(Entity *other)
{
	Target *t;

	t = getTargetByName("SORCEROR_PLAYER_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	player.targetX = t->x;
	player.targetY = t->y;

	calculatePath(player.x, player.y, player.targetX, player.targetY, &player.dirX, &player.dirY);

	player.flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	playSoundToMap("sound/common/spell", EDGAR_CHANNEL, self->x, self->y, 0);

	self->inUse = FALSE;
}

static void arrowTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		damage = 0;
	}
}

void drawDisintegrationLine(int x1, int y1, int x2, int y2, Colour colour1, Colour colour2, Colour colour3)
{
	int speed;
	float dirX, dirY, x, y, xx, yy;

	speed = 8;

	calculatePath(x1, y1, x2, y2, &dirX, &dirY);

	dirX *= speed;
	dirY *= speed;

	x = x1;
	y = y1;

	if (fabs(dirX) > fabs(dirY))
	{
		while (!(fabs(x2 - x) <= fabs(dirX)))
		{
			yy = (prand() % 8) * (prand() % 2 == 0 ? -1 : 1);

			drawColouredLine(x, y, x + dirX, y + dirY + yy, colour1, colour2, colour3);

			x += dirX;
			y += dirY + yy;

			drawColouredLine(x, y, x + dirX, y + dirY - yy, colour1, colour2, colour3);

			x += dirX;
			y += dirY - yy;
		}
	}

	else
	{
		while (!(fabs(y2 - y) <= fabs(dirY)))
		{
			xx = (prand() % 8) * (prand() % 2 == 0 ? -1 : 1);

			drawColouredLine(x, y, x + dirX + xx, y + dirY, colour1, colour2, colour3);

			x += dirX + xx;
			y += dirY;

			drawColouredLine(x, y, x + dirX - xx, y + dirY, colour1, colour2, colour3);

			x += dirX - xx;
			y += dirY;
		}
	}
}
