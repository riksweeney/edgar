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
#include "../entity.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void headDie(void);
static void init(void);
static void headWait(void);
static void lookForPlayer(void);
static void bodyTakeDamage(Entity *, int);
static void headTakeDamage(Entity *, int);
static void alignBodyToHead(void);
static void createBody(Entity *, Entity *);
static void headChangeDirection(Entity *);
static void bodyWait(void);
static void biteWait(void);
static void headBiteInit(void);
static void headBite(void);
static void headBiteReturn(void);
static void headBiteReactToBlock(Entity *);
static void creditsMove(void);
static void die(void);

Entity *addFlyTrap(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fly Trap");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fly Trap Head");
	}

	loadProperties("enemy/fly_trap_head", e);

	e->action = &headWait;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &headDie;
	e->touch = &entityTouch;
	e->takeDamage = &headTakeDamage;
	e->reactToBlock = &headChangeDirection;

	e->creditsAction = &headWait;

	e->head = self;

	self->head = e;

	createBody(e, self);

	e->face = self->face;

	e->dirX = self->dirX;

	e->health = self->health;

	self->action = &lookForPlayer;

	self->creditsAction = &creditsMove;
}

static void lookForPlayer()
{
	moveLeftToRight();

	self->thinkTime--;

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */

		if (collision(self->x + (self->face == LEFT ? -100 : self->w + 64), self->y, 36, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			self->thinkTime = 60;

			self->action = &biteWait;

			self->mental = 1;

			playSoundToMap("sound/enemy/mouth_stalk/hiss", -1, self->x, self->y, 0);
		}
	}
}

static void biteWait()
{
	checkToMap(self);

	if (self->mental == 0)
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->action = &lookForPlayer;
	}
}

static void headDie()
{
	Entity *e;

	e = self;

	self = self->target;

	while (self != NULL)
	{
		self->die();

		self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		self->dirY = ITEM_JUMP_HEIGHT;

		self = self->target;
	}

	self = e;

	entityDieNoDrop();

	self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
	self->dirY = ITEM_JUMP_HEIGHT;
}

static void headWait()
{
	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	self->startY = self->y;

	self->endX += 3;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->endX)) * 4;

	checkToMap(self);

	alignBodyToHead();

	if (self->head->health <= 0)
	{
		self->die();
	}

	if (self->head->mental == 1)
	{
		self->thinkTime = 30;

		self->maxThinkTime = 1 + prand() % 3;

		self->action = &headBiteInit;
	}

	self->damage = self->head->damage;

	self->face = self->head->face;

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void headBiteInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->startX = self->x;
		self->startY = self->y;

		self->targetX = player.x;
		self->targetY = player.y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 16;
		self->dirY *= 16;

		self->action = &headBite;

		self->reactToBlock = &headBiteReactToBlock;
	}

	if (self->head->health <= 0)
	{
		self->die();
	}

	alignBodyToHead();

	self->damage = self->head->damage;

	self->face = self->head->face;

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}
}

static void headBite()
{
	checkToMap(self);

	if (atTarget() || self->dirX == 0 || self->dirY == 0)
	{
		self->targetX = self->startX;
		self->targetY = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 16;
		self->dirY *= 16;

		self->flags |= UNBLOCKABLE;

		self->action = &headBiteReturn;
	}

	if (self->head->health <= 0)
	{
		self->die();
	}

	alignBodyToHead();

	self->damage = self->head->damage;

	self->face = self->head->face;

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}
}

static void headBiteReactToBlock(Entity *other)
{
	self->dirX = 0;
}

static void headBiteReturn()
{
	float dirX, dirY;

	dirX = self->dirX;
	dirY = self->dirY;

	checkToMap(self);

	if (atTarget() || self->dirX != dirX || self->dirY != dirY)
	{
		self->flags &= ~UNBLOCKABLE;

		self->x = self->startX;
		self->y = self->startY;

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->dirX = self->face == LEFT ? -self->head->speed : self->head->speed;

			self->head->thinkTime = 120;

			self->head->mental = 0;

			self->action = &headWait;

			self->reactToBlock = &headChangeDirection;
		}

		else
		{
			self->action = &headBiteInit;
		}
	}

	if (self->head->health <= 0)
	{
		self->die();
	}

	alignBodyToHead();

	self->damage = self->head->damage;

	self->face = self->head->face;

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}
}

static void createBody(Entity *trapHead, Entity *trapBase)
{
	int i;
	Entity **body, *head;

	trapHead->x = trapHead->endX;
	trapHead->y = trapHead->endY;

	body = malloc(trapHead->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Fly Trap body...", trapHead->mental * (int)sizeof(Entity *));
	}

	for (i=trapHead->mental-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			showErrorAndExit("No free slots to add a Fly Trap body part");
		}

		loadProperties("enemy/fly_trap_body", body[i]);

		body[i]->x = trapHead->x;
		body[i]->y = trapHead->y;

		body[i]->action = &bodyWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;
		body[i]->takeDamage = &bodyTakeDamage;

		body[i]->creditsAction = &bodyWait;

		body[i]->type = ENEMY;

		body[i]->health = trapBase->health;

		setEntityAnimation(body[i], "STAND");
	}

	/* Recreate the head so that it's on top */

	head = getFreeEntity();

	if (head == NULL)
	{
		showErrorAndExit("No free slots to add a Fly Trap head");
	}

	*head = *trapHead;

	trapHead->inUse = FALSE;

	trapHead = head;

	/* Link the sections */

	for (i=trapHead->mental-1;i>=0;i--)
	{
		if (i == 0)
		{
			trapHead->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];

			if (i == trapHead->mental - 1)
			{
				body[i]->endX = trapBase->x;
				body[i]->endY = trapBase->y;
			}
		}

		body[i]->head = trapHead;
	}

	free(body);

	trapHead->action = &headWait;

	trapHead->creditsAction = &headWait;
}

static void alignBodyToHead()
{
	float x, y, partDistanceX, partDistanceY, endX, endY;
	Entity *e;

	y = self->y;

	if (self->face == LEFT)
	{
		x = self->x + self->w - self->target->w;

		endX = self->head->x + self->head->offsetX;
	}

	else
	{
		x = self->x;

		endX = self->head->x + self->head->offsetX;
	}

	endY = self->head->y + self->head->offsetY;

	partDistanceX = endX - x;

	partDistanceY = fabs(endY - self->y);

	partDistanceX /= self->mental;
	partDistanceY /= self->mental;

	e = self->target;

	while (e != NULL)
	{
		x += partDistanceX;
		y += partDistanceY;

		e->x = (e->target == NULL ? endX : x);
		e->y = (e->target == NULL ? endY : y);

		e = e->target;
	}
}

static void bodyTakeDamage(Entity *other, int damage)
{
	Entity *temp = self;

	self = self->head->head; /* Get the head, then the base */

	self->takeDamage(other, damage);

	self = temp;
}

static void headTakeDamage(Entity *other, int damage)
{
	Entity *temp = self;

	self = self->head;

	self->takeDamage(other, damage);

	self = temp;
}

static void headChangeDirection(Entity *other)
{
	self->head->dirX = 0;
}

static void bodyWait()
{
	self->damage = self->head->damage;

	self->face = self->head->face;

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
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
}

static void die()
{
	playSoundToMap("sound/enemy/jumping_slime/slime_die", -1, self->x, self->y, 0);

	entityDie();
}
