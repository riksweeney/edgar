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

#include "../graphics/animation.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../audio/audio.h"

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
static void addSwordSwing(void);
static void swordLookForPlayer(void);
static void attackPlayer(void);
static void slashInit(void);
static void slash(void);
static void addSwordSwing(void);
static void swordSwingWait(void);
static void swordSwingAttack(void);
static void swordSwingAttackFinish(void);
static void swordReactToBlock(Entity *);
static void creditsMove(void);

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

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->health == 0)
	{
		self->action = &die;
	}

	else
	{
		if (strcmpignorecase(self->name, "enemy/arrow_skeleton") == 0)
		{
			self->action = &arrowLookForPlayer;
		}

		else
		{
			addSwordSwing();

			self->action = &swordLookForPlayer;
		}
	}
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	loadProperties(prand() % 2 == 0 ? "enemy/arrow_skeleton" : "enemy/sword_skeleton", self);

	snprintf(name, sizeof(name), "%s_piece", self->name);

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	dropRandomItem(self->x + self->w / 2, self->y);

	/*playSoundToMap("sound/enemy/centurion/centurion_die.ogg", -1, self->x, self->y, 0);*/

	for (i=0;i<6;i++)
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

	self->thinkTime = 300;
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		self->mental = 6;

		if (self->health != 0)
		{
			self->action = &reform;
		}
	}
}

static void reform()
{
	if (self->mental == 0)
	{
		self->thinkTime = 30;

		self->action = &reformFinish;

		self->creditsAction = &creditsReformFinish;
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
			self->action = &arrowLookForPlayer;
		}

		else
		{
			addSwordSwing();

			self->action = &swordLookForPlayer;
		}

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->damage = 1;

		self->takeDamage = &entityTakeDamageNoFlinch;

		setEntityAnimation(self, "STAND");

		self->creditsAction = &creditsMove;
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

	if ((self->flags & ON_GROUND) && !(self->flags & FLY))
	{
		self->dirX = 0;
	}

	checkToMap(self);
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
			if (self->head->mental <= 0)
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

		if (collision(self->x + (self->face == LEFT ? -300 : self->w), self->y, 300, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			facePlayer();

			self->thinkTime = 30;

			self->action = &readyArrow;
		}
	}
}

static void readyArrow()
{
	self->dirX = 0;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "FIRE_ARROW");

		self->animationCallback = &fireArrow;

		self->thinkTime = 15;

		self->action = &fireArrowWait;
	}

	checkToMap(self);
}

static void fireArrowWait()
{
	checkToMap(self);
}

static void fireArrow()
{
	Entity *e;

	e = addProjectile("weapon/flaming_arrow", self, self->x + (self->face == RIGHT ? 0 : self->w), self->y + 27, self->face == RIGHT ? 12 : -12, 0);

	e->damage = 1;

	if (e->face == LEFT)
	{
		e->x -= e->w;
	}

	playSoundToMap("sound/enemy/fireball/fireball.ogg", -1, self->x, self->y, 0);

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

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental--;

		if (self->mental <= 0)
		{
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

	if ((self->face == LEFT && abs(self->x - (player.x + player.w)) < 16) || (self->face == RIGHT && abs(player.x - (self->x + self->w)) < 16))
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

		if (self->thinkTime <= 0)
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

		setEntityAnimation(self, "ATTACK_1");

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void slash()
{
	if (self->mental == 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->endY--;

			if (self->endY <= 0)
			{
				self->thinkTime = 0;
			}

			else
			{
				self->thinkTime = 30;
			}

			self->thinkTime = 300;

			self->action = &attackPlayer;
		}
	}

	checkToMap(self);
}

static void addSwordSwing()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Skeleton Sword");
	}

	loadProperties("enemy/sword_skeleton_sword", e);

	e->x = 0;
	e->y = 0;

	e->action = &swordSwingWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

	e->flags |= ATTACKING;

	setEntityAnimation(e, "STAND");
}

static void swordSwingWait()
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

	if (self->head->flags & ATTACKING)
	{
		self->action = &swordSwingAttack;

		self->reactToBlock = &swordReactToBlock;

		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &swordSwingAttackFinish;

		playSoundToMap("sound/edgar/swing.ogg", -1, self->x, self->y, 0);
	}

	else
	{
		self->damage = 0;

		self->flags |= NO_DRAW;
	}

	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
	}
}

static void swordSwingAttack()
{
	self->flags &= ~NO_DRAW;

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

static void swordSwingAttackFinish()
{
	self->damage = 0;

	self->flags |= NO_DRAW;

	setEntityAnimation(self, "STAND");

	self->action = &swordSwingWait;

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
