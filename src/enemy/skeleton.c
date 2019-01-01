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
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../item/item.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void init(void);
static void die(void);
static void dieWait(void);
static void reform(void);
static void reformFinish(void);
static void pieceWait(void);
static void pieceReform(void);
static void pieceFallout(void);
static void arrowLookForPlayer(void);
static void readyArrow(void);
static void fireArrow(void);
static void fireArrowWait(void);
static void fireArrowFinish(void);
static void swordLookForPlayer(void);
static void attackPlayer(void);
static void slashInit(void);
static void slash(void);
static void addSword(void);
static void swordWait(void);
static void swordAttack(void);
static void swordAttackFinish(void);
static void swordReactToBlock(Entity *);
static void addBow(void);
static void bowWait(void);
static void creditsMove(void);
static void creditsMove2(void);
static void creditsDie(void);

Entity *addSkeleton(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Skeleton");
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
	if (self->health == 0)
	{
		self->damage = -1;

		self->action = &die;
	}

	else
	{
		if (strcmpignorecase(self->name, "enemy/arrow_skeleton") == 0)
		{
			addBow();

			self->action = &arrowLookForPlayer;
		}

		else
		{
			addSword();

			self->action = &swordLookForPlayer;
		}
	}

	self->mental = 150 + prand() % 210;

	self->creditsAction = &creditsMove;
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	i = self->damage;

	loadProperties(prand() % 2 == 0 ? "enemy/arrow_skeleton" : "enemy/sword_skeleton", self);

	snprintf(name, sizeof(name), "%s_piece", self->name);

	self->mental = 0;

	self->damage = i;

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->action = &pieceWait;

		e->creditsAction = &pieceWait;

		e->fallout = &pieceFallout;

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->head = self;

		self->mental++;
	}

	if (self->damage >= 0)
	{
		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		dropRandomItem(self->x + self->w / 2, self->y);

		playSoundToMap("sound/enemy/skeleton/skeleton_die", -1, self->x, self->y, 0);

		self->thinkTime = 30 * 60;
	}

	self->endX = self->damage;

	self->damage = 0;

	self->health = 0;

	self->dirX = 0;

	self->flags &= ~FLY;

	self->flags |= NO_DRAW;

	self->takeDamage = NULL;

	self->action = &dieWait;

	self->creditsAction = &dieWait;
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		if (self->health != 0)
		{
			playSoundToMap("sound/enemy/skeleton/skeleton_resurrect", -1, self->x, self->y, 0);

			self->action = &reform;

			self->creditsAction = &reform;
		}
	}

	checkToMap(self);
}

static void reform()
{
	if (self->mental == 0)
	{
		self->thinkTime = 30;

		self->action = &reformFinish;

		self->creditsAction = &reformFinish;
	}
}

static void reformFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health = self->maxHealth;

		self->flags &= ~NO_DRAW;

		if (strcmpignorecase(self->name, "enemy/arrow_skeleton") == 0)
		{
			addBow();

			self->action = &arrowLookForPlayer;
		}

		else
		{
			addSword();

			self->action = &swordLookForPlayer;
		}

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->damage = 1;

		self->takeDamage = &entityTakeDamageNoFlinch;

		setEntityAnimation(self, "STAND");

		self->animationCallback = NULL;

		self->flags &= ~ATTACKING;

		self->creditsAction = &creditsMove2;
	}
}

static void pieceWait()
{
	if (self->head->health != 0)
	{
		self->action = &pieceReform;

		self->creditsAction = &pieceReform;

		if (self->face == LEFT)
		{
			self->targetX = self->head->x + self->head->w - self->w - self->offsetX;
		}

		else
		{
			self->targetX = self->head->x + self->offsetX;
		}

		self->targetY = self->head->y + self->offsetY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->flags |= FLY;

		self->touch = NULL;
	}

	else
	{
		checkToMap(self);
	}

	if ((self->flags & ON_GROUND) && !(self->flags & FLY))
	{
		self->dirX = 0;
	}
}

static void pieceReform()
{
	if (atTarget())
	{
		if (self->mental == 0)
		{
			self->mental = 1;

			self->head->mental--;
		}

		else
		{
			if (!(self->head->flags & NO_DRAW))
			{
				self->inUse = FALSE;
			}
		}
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void pieceFallout()
{
	self->dirX = 0;
	self->dirY = 0;

	self->x = self->head->x;
	self->y = self->head->y;
}

static void arrowLookForPlayer()
{
	setEntityAnimation(self, "WALK");

	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */

		if (collision(self->x + (self->face == LEFT ? -200 : self->w), self->y, 200, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			self->mental = 1 + prand() % 3;

			facePlayer();

			self->action = &readyArrow;
		}
	}
}

static void readyArrow()
{
	self->flags |= ATTACKING;

	setEntityAnimation(self, "FIRE_ARROW");

	self->animationCallback = &fireArrow;

	self->thinkTime = 15;

	self->action = &fireArrowWait;

	checkToMap(self);
}

static void fireArrowWait()
{
	checkToMap(self);
}

static void fireArrow()
{
	Entity *e;

	e = addProjectile("weapon/normal_arrow", self, self->x + (self->face == RIGHT ? 0 : self->w), self->y + self->offsetY, self->face == RIGHT ? 12 : -12, 0);

	e->damage = 1;

	if (e->face == LEFT)
	{
		e->x -= e->w;
	}

	playSoundToMap("sound/edgar/arrow", -1, self->x, self->y, 0);

	e->reactToBlock = &bounceOffShield;

	e->face = self->face;

	e->flags |= FLY;

	setEntityAnimation(self, "FIRE_ARROW_FINISH");

	self->animationCallback = &fireArrowFinish;

	checkToMap(self);
}

static void fireArrowFinish()
{
	self->action = &fireArrowFinish;

	setEntityAnimation(self, "STAND");

	self->flags &= ~ATTACKING;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;

			self->action = &arrowLookForPlayer;

			self->thinkTime = 180;
		}

		else
		{
			self->action = &readyArrow;
		}
	}

	checkToMap(self);
}

static void swordLookForPlayer()
{
	setEntityAnimation(self, "WALK");

	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */

		if (collision(self->x + (self->face == LEFT ? -240 : self->w), self->y, 240, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->thinkTime = 300;

			self->action = &attackPlayer;
		}
	}
}

static void attackPlayer()
{
	/* Get close to the player */

	facePlayer();

	if ((self->face == LEFT && abs(self->x - (player.x + player.w)) < 24) || (self->face == RIGHT && abs(player.x - (self->x + self->w)) < 24))
	{
		setEntityAnimation(self, "STAND");

		self->dirX = 0;

		self->mental = 3;

		self->thinkTime = 30;

		self->action = &slashInit;
	}

	else
	{
		setEntityAnimation(self, "WALK");

		self->thinkTime--;

		if (self->thinkTime <= 0 || isAtEdge(self) == TRUE)
		{
			self->face = self->face == LEFT ? RIGHT : LEFT;

			self->action = &swordLookForPlayer;

			self->thinkTime = 240;
		}

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	checkToMap(self);
}

static void slashInit()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->thinkTime = 60;

		self->action = &slash;

		setEntityAnimation(self, "SLASH");

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void slash()
{
	if (self->mental <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = &swordLookForPlayer;

			self->dirX = self->face == LEFT ? -self->speed : self->speed;
		}
	}

	checkToMap(self);
}

static void addBow()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Skeleton Bow");
	}

	loadProperties("enemy/skeleton_bow", e);

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

	e->action = &bowWait;

	e->draw = &drawLoopingAnimationToMap;
	e->die = NULL;
	e->takeDamage = NULL;

	e->creditsAction = &bowWait;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, getAnimationTypeAtIndex(self));
}

static void bowWait()
{
	if (self->head->flags & ATTACKING)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

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

	if (self->head->health <= 0 || self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void addSword()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Skeleton Sword");
	}

	loadProperties("enemy/skeleton_sword", e);

	e->x = 0;
	e->y = 0;

	e->action = &swordWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = NULL;
	e->takeDamage = NULL;

	e->creditsAction = &swordWait;

	e->type = ENEMY;

	e->head = self;

	e->flags |= ATTACKING;

	setEntityAnimation(e, "STAND");
}

static void swordWait()
{
	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

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

	if (self->head->flags & ATTACKING)
	{
		self->action = &swordAttack;

		self->reactToBlock = &swordReactToBlock;

		setEntityAnimation(self, "SLASH");

		self->animationCallback = &swordAttackFinish;

		playSoundToMap("sound/edgar/swing", -1, self->x, self->y, 0);
	}

	else
	{
		self->damage = 0;
	}

	if (self->head->health <= 0 || self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void swordAttack()
{
	self->damage = 1;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void swordAttackFinish()
{
	self->damage = 0;

	setEntityAnimation(self, "STAND");

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	self->action = &swordWait;

	self->head->mental--;

	if (self->head->mental == 0)
	{
		self->head->flags &= ~ATTACKING;
	}
}

static void swordReactToBlock(Entity *other)
{
	self->damage = 0;
}

static void creditsDie()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	snprintf(name, sizeof(name), "%s_piece", self->name);

	self->mental = 0;

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->action = &pieceWait;

		e->creditsAction = &pieceWait;

		e->fallout = &pieceFallout;

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->head = self;

		self->mental++;
	}

	playSoundToMap("sound/enemy/skeleton/skeleton_die", -1, self->x, self->y, 0);

	e = getEntityByName("enemy/ghost");

	e->health++;

	self->endX = self->damage;

	self->damage = 0;

	self->health = 0;

	self->dirX = 0;

	self->flags &= ~FLY;

	self->flags |= NO_DRAW;

	self->takeDamage = NULL;

	self->action = &dieWait;

	self->creditsAction = &dieWait;
}

static void creditsMove()
{
	self->face = RIGHT;

	setEntityAnimation(self, "WALK");

	self->dirX = self->speed;

	checkToMap(self);

	self->mental--;

	if (self->mental <= 0)
	{
		self->creditsAction = &creditsDie;
	}
}

static void creditsMove2()
{
	self->face = RIGHT;

	setEntityAnimation(self, "WALK");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
