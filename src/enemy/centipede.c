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
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void init(void);
static void touch(Entity *);
static void headMove(void);
static void addSegments(void);
static void segmentMove(void);
static void segmentInit(void);
static void reactToBlock(Entity *);
static void segmentTakeDamage(Entity *, int);
static void becomeHead(void);
static void greenTouch(Entity *);
static void becomeGreen(void);
static void creditsMove(void);
static void die(void);

Entity *addCentipede(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Centipede");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &touch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &reactToBlock;

	e->creditsAction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	addSegments();

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->action = &headMove;

	if (self->mental >= 2)
	{
		self->touch = &greenTouch;
	}

	self->creditsAction = &creditsMove;
}

static void headMove()
{
	int currentFrame, health;

	if (self->mental >= 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 300)
		{
			if (self->thinkTime % 15 == 0)
			{
				currentFrame = self->currentFrame;

				if (self->mental < 4)
				{
					setEntityAnimation(self, self->mental == 2 ? "CUSTOM_1" : "CUSTOM_2");

					self->mental = self->mental == 2 ? 4 : 5;
				}

				else
				{
					setEntityAnimation(self, "STAND");

					self->mental = self->mental == 4 ? 2 : 3;
				}

				self->currentFrame = currentFrame;
			}

			if (self->thinkTime <= 0)
			{
				currentFrame = self->currentFrame;

				health = self->health;

				loadProperties(self->mental % 2 == 0 ? "enemy/yellow_centipede" : "enemy/red_centipede", self);

				self->health = health;

				self->mental = self->mental % 2;

				self->touch = &touch;

				self->currentFrame = currentFrame;
			}
		}
	}

	moveLeftToRight();
}

static void die()
{
	playSoundToMap("sound/enemy/centipede/centipede_die", -1, self->x, self->y, 0);

	entityDie();
}

static void addSegments()
{
	char name[MAX_VALUE_LENGTH];
	int i, frameCount, mental;
	Entity *e, *prev;

	prev = self;

	if (strcmpignorecase(self->name, "enemy/green_centipede") == 0)
	{
		mental = 2;
	}

	else if (strcmpignorecase(self->name, "enemy/red_centipede") == 0)
	{
		mental = 1;
	}

	else
	{
		mental = 0;
	}

	snprintf(name, MAX_VALUE_LENGTH, "%s_segment", self->name);

	for (i=0;i<5;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Centipede Segment");
		}

		loadProperties(name, e);

		e->action = &segmentInit;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->takeDamage = &segmentTakeDamage;
		e->die = &entityDieNoDrop;

		e->creditsAction = &segmentInit;

		e->type = ENEMY;

		e->face = self->face;

		e->target = prev;

		e->head = self;

		e->speed = self->speed;

		setEntityAnimation(e, "STAND");

		frameCount = getFrameCount(e);

		e->currentFrame = prand() % frameCount;

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y;

		e->mental = mental;

		STRNCPY(e->objectiveName, self->name, sizeof(e->objectiveName));

		prev = e;

		if (i == 0)
		{
			self->head = e;
		}

		if (self->flags & SPAWNED_IN)
		{
			e->flags |= SPAWNED_IN;

			e->spawnTime = self->spawnTime;
		}
	}

	self->mental = mental;
}

static void segmentInit()
{
	if (self->face == LEFT && fabs(self->target->x - self->x) >= self->target->w)
	{
		self->action = &segmentMove;

		self->creditsAction = &creditsMove;
	}

	else if (self->face == RIGHT && fabs(self->target->x - self->x) >= self->w)
	{
		self->action = &segmentMove;

		self->creditsAction = &creditsMove;
	}

	checkToMap(self);
}

static void segmentMove()
{
	int currentFrame, health;

	if (self->thinkTime <= 0 && self->target->mental >= 2)
	{
		currentFrame = self->currentFrame;

		health = self->health;

		loadProperties("enemy/green_centipede_segment", self);

		self->health = health;

		self->mental = self->target->mental;

		self->touch = &greenTouch;

		self->thinkTime = 60 * 30;

		self->currentFrame = currentFrame;
	}

	if (self->mental >= 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 300)
		{
			if (self->thinkTime % 15 == 0)
			{
				currentFrame = self->currentFrame;

				if (self->mental < 4)
				{
					setEntityAnimation(self, self->mental == 2 ? "CUSTOM_1" : "CUSTOM_2");

					self->mental = self->mental == 2 ? 4 : 5;
				}

				else
				{
					setEntityAnimation(self, "STAND");

					self->mental = self->mental == 4 ? 2 : 3;
				}

				self->currentFrame = currentFrame;
			}

			if (self->thinkTime <= 0)
			{
				currentFrame = self->currentFrame;

				health = self->health;

				loadProperties(self->mental % 2 == 0 ? "enemy/yellow_centipede_segment" : "enemy/red_centipede_segment", self);

				self->health = health;

				self->mental = self->mental % 2;

				self->touch = &entityTouch;

				self->currentFrame = currentFrame;
			}
		}
	}

	else if (self->mental == 0 && self->head->health <= 0)
	{
		self->health = 0;

		entityDie();
	}

	if (self->target->health <= 0)
	{
		if (self->mental == 1)
		{
			self->action = &becomeHead;
		}

		else
		{
			self->health = 0;

			entityDieNoDrop();
		}
	}

	/* If segment is facing the same way as the target then just move with it */

	if (self->face == LEFT && self->x + self->w <= self->target->x)
	{
		self->x = self->target->x - self->w;

		self->face = self->target->face;
	}

	else if (self->face == RIGHT && self->x >= self->target->x + self->target->w)
	{
		self->x = self->target->x + self->target->w;

		self->face = self->target->face;
	}

	else
	{
		self->dirX = self->face == LEFT ? -self->target->speed : self->target->speed;
	}

	checkToMap(self);
}

static void becomeHead()
{
	int x, y;

	x = self->x + self->w;
	y = self->y + self->h;

	loadProperties(self->objectiveName, self);

	self->action = &moveLeftToRight;
	self->die = &entityDie;
	self->touch = &touch;
	self->takeDamage = &entityTakeDamageNoFlinch;
	self->reactToBlock = &reactToBlock;
	self->pain = &enemyPain;

	self->flags &= ~UNBLOCKABLE;

	self->x = x;

	self->y = y - self->h;

	/* Always walk away from the player */

	if (player.x > self->x)
	{
		if (self->face == RIGHT)
		{
			self->x = x - self->w;
		}

		self->face = LEFT;
	}

	else
	{
		self->face = RIGHT;
	}
}

static void reactToBlock(Entity *other)
{
	self->endX = self->x;

	changeDirection(NULL);
}

static void segmentTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			enemyPain();
		}

		else
		{
			self->damage = 0;

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void touch(Entity *other)
{
	if (self->mental < 2 && strcmpignorecase(other->name, "item/spore") == 0)
	{
		other->inUse = FALSE;

		self->action = &becomeGreen;
	}

	else
	{
		entityTouch(other);
	}
}

static void becomeGreen()
{
	int currentFrame, health;

	currentFrame = self->currentFrame;

	health = self->health;

	loadProperties("enemy/green_centipede", self);

	self->health = health;

	self->mental = self->mental == 0 ? 2 : 3;

	self->touch = &greenTouch;

	self->thinkTime = 60 * 30;

	self->action = &headMove;

	self->currentFrame = currentFrame;
}

static void greenTouch(Entity *other)
{
	float y1, y2;

	if (other->type == PLAYER && other->dirY > 0)
	{
		y1 = other->y + other->h - other->dirY;
		y2 = other->y + other->h;

		if (y1 <= self->y + self->box.y && y2 > self->y + self->box.y)
		{
			other->y = self->y + self->box.y - other->h;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;
		}
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
