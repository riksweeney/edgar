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
#include "../hud.h"
#include "../item/ice_cube.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "rock.h"
#include "thunder_cloud.h"

extern Entity *self, player;

static void hover(void);
static void lookForPlayer(void);
static void dartDownInit(void);
static void dartDown(void);
static void dartDownFinish(void);
static void dartReactToBlock(Entity *);
static void castFireInit(void);
static void castFire(void);
static void castFinish(void);
static void fireDrop(void);
static void fireMove(void);
static void fireBlock(Entity *);
static void castIceInit(void);
static void iceTouch(Entity *);
static void iceFallout(void);
static void castLightningBolt(void);
static void lightningBolt(void);
static void takeDamage(Entity *, int);
static void creditsMove(void);

Entity *addBook(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Book");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	float dirX;

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

	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;
	}

	if (self->mental == 3 && self->thinkTime <= 0)
	{
		self->action = &castIceInit;

		self->thinkTime = 60;

		self->dirX = 0;
	}

	else if (player.health > 0 && self->thinkTime == 0 && prand() % 20 == 0)
	{
		self->thinkTime = 0;

		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, 200, player.x, player.y, player.w, player.h) == 1)
		{
			switch (self->mental)
			{
				case 0: /* Ram player */
					self->action = &dartDownInit;

					self->thinkTime = 60;
				break;

				case 1: /* Cast fire */
					self->action = &castFireInit;

					self->thinkTime = 60;
				break;

				case 2: /* Cast lightning */
					self->action = &castLightningBolt;

					self->thinkTime = 60;
				break;
			}

			self->dirX = 0;
		}
	}

	hover();
}

static void dartDownInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = player.x;
		self->targetY = player.y + player.h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed * 10;
		self->dirY *= self->speed * 10;

		self->face = self->dirX < 0 ? LEFT : RIGHT;

		self->action = &dartDown;

		self->reactToBlock = &dartReactToBlock;
	}

	hover();
}

static void dartDown()
{
	if (self->dirY == 0 || self->dirX == 0)
	{
		self->thinkTime = 30;

		self->flags &= ~FLY;

		self->dirX = self->face == RIGHT ? -3 : 3;

		self->dirY = -5;

		self->action = &dartDownFinish;
	}

	checkToMap(self);
}

static void dartDownFinish()
{
	if (self->thinkTime == 0)
	{
		self->dirX = 0;

		self->dirY = -self->speed;

		self->flags |= FLY;

		self->thinkTime = -1;
	}

	else if (self->thinkTime < 0)
	{
		if (self->dirY == 0 || self->y <= self->startY)
		{
			self->thinkTime = 60;

			self->startY = self->y;

			self->action = &lookForPlayer;

			self->dirX = self->dirY = 0;

			self->reactToBlock = &changeDirection;
		}
	}

	else
	{
		self->thinkTime--;

		if ((self->flags & ON_GROUND) || (self->standingOn != NULL))
		{
			self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
		}
	}

	checkToMap(self);
}

static void dartReactToBlock(Entity *other)
{
	self->dirX = 0;

	self->thinkTime = 60;
}

static void hover()
{
	self->startX += 5;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 4;
}

static void castFireInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->endX = 5;

		self->action = &castFire;

		playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);
	}

	hover();
}

static void castFire()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->action = &fireDrop;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->reactToBlock = &fireBlock;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 600;

		e->flags |= DO_NOT_PERSIST;

		setEntityAnimation(e, "DOWN");

		self->endX--;

		if (self->endX <= 0)
		{
			self->thinkTime = 60;

			self->action = &castFinish;
		}

		else
		{
			self->thinkTime = 3;
		}
	}
}

static void castFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
        self->dirX = self->face == LEFT ? -self->speed : self->speed;

		if (self->mental == 3)
		{
			self->thinkTime = 120 + prand() % 120;
		}

		else
		{
			self->thinkTime = 60;
		}

		self->action = &lookForPlayer;
	}
}

static void fireDrop()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	else if (self->flags & ON_GROUND)
	{
		self->thinkTime = 600;

		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->action = &fireMove;
	}

	checkToMap(self);
}

static void fireMove()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->dirX == 0 || self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void fireBlock(Entity *other)
{
	self->dirX = 0;
}

static void castIceInit()
{
	float i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=-3;i<=3;i+=1.5f)
		{
			if (i == 0)
			{
				continue;
			}

			e = addProjectile("enemy/ice", self, 0, 0, i, ITEM_JUMP_HEIGHT);

			e->x = self->x + self->w / 2;
			e->y = self->y + self->h / 2;

			e->x -= e->w / 2;
			e->y -= e->h / 2;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &iceTouch;
			e->fallout = &iceFallout;

			e->face = self->face;

			setEntityAnimation(e, "STAND");
		}

		self->thinkTime = 60;

		self->action = &castFinish;
	}

	hover();
}

static void iceFallout()
{
	Entity *e;

	if (self->environment == WATER)
	{
		e = addIceCube(self->x + self->w / 2, self->y + self->h / 2, "item/ice_cube");

		e->x -= e->w / 2;
		e->y -= e->h / 2;
	}

	self->inUse = FALSE;
}

static void iceTouch(Entity *other)
{
	if (other->type == PLAYER && other->element != ICE && !(other->flags & INVULNERABLE) && other->health > 0)
	{
		setPlayerFrozen(120);

		self->inUse = FALSE;
	}
}

static void castLightningBolt()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add lightning");
		}

		loadProperties("enemy/lightning", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapCeiling(e->targetX, self->y);

		e->startY = e->targetY;
		e->endY   = getMapFloor(e->targetX, e->targetY);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		e->head = self;

		e->face = RIGHT;

		e->action = &lightningBolt;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= FLY|DO_NOT_PERSIST;

		self->thinkTime = 30;

		self->action = &castFinish;
	}
}

static void lightningBolt()
{
	int i, middle;
	Entity *e;

	self->flags |= NO_DRAW;

	self->thinkTime--;

	middle = -1;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/enemy/thunder_cloud/lightning", -1, self->targetX, self->startY, 0);

		for (i=self->startY;i<self->endY;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add lightning");
			}

			loadProperties("enemy/lightning", e);

			setEntityAnimation(e, "STAND");

			if (i == self->startY)
			{
				middle = self->targetX + self->w / 2 - e->w / 2;
			}

			e->x = middle;
			e->y = i;

			e->action = &lightningWait;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->currentFrame = prand() % 6;

			e->face = RIGHT;

			e->thinkTime = 15;
		}

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if ((self->mental == 2 && other->element == LIGHTNING) || (self->mental == 1 && other->element == FIRE))
	{
		if (self->flags & INVULNERABLE)
		{
			return;
		}

		if (damage != 0)
		{
			self->health += damage;

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}

			if (prand() % 5 == 0)
			{
				setInfoBoxMessage(90, 255, 255, 255, _("The damage from this weapon is being absorbed..."));
			}
		}
	}

	else
	{
		entityTakeDamageNoFlinch(other, damage);
	}
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	hover();
}
