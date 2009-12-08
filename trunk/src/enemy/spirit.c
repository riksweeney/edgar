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
#include "../audio/audio.h"
#include "../entity.h"
#include "../geometry.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../system/properties.h"
#include "../collisions.h"
#include "../world/target.h"
#include "../player.h"
#include "../event/trigger.h"

extern Entity *self, player;

static void hover(void);
static void touch(Entity *);
static void lookForPlayer(void);
static void teleportPlayer(void);
static void die(void);

Entity *addSpirit(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spirit");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &touch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	float dirX;

	if (self->x <= self->startX || self->x >= self->endX)
	{
		self->x = (self->x <= self->startX ? self->startX : self->endX);

		self->dirX = 0;
	}

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	self->face = self->dirX > 0 ? RIGHT : LEFT;

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->face = self->face == LEFT ? RIGHT : LEFT;
	}

	hover();
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		teleportPlayer();

		self->action = &lookForPlayer;
	}
}

static void teleportPlayer()
{
	Target *t = getTargetByName("SPIRIT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Spirit cannot find target");
	}

	player.targetX = t->x;
	player.targetY = t->y;

	calculatePath(player.x, player.y, player.targetX, player.targetY, &player.dirX, &player.dirY);

	player.flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	playSoundToMap("sound/common/teleport.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

	activateEntitiesWithObjectiveName("SPIRIT_POINT", TRUE);
}

static void die()
{

}

static void hover()
{
	self->thinkTime += 5;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->thinkTime)) * 4;
}
