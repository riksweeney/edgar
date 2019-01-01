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
#include "../event/global_trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void init(void);
static void entityWait(void);
static void die(void);
static void tongueAttackInit(void);
static void tongueAttack(void);
static void tongueAttackWait(void);
static void tongueAttackInit(void);
static void tongueMove(void);
static void tongueReturn(void);
static int drawTongue(void);
static void tongueTouch(Entity *);
static void frogTouch(Entity *);
static void grabPause(void);
static void addExitTrigger(Entity *);
static void creditsMove(void);

Entity *addFrog(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Frog");
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
	e->fallout = &die;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *e;

	if (strlen(self->objectiveName) != 0)
	{
		e = getEntityByName(self->objectiveName);

		if (e != NULL)
		{
			self->target = e;

			self->target->flags |= NO_DRAW;
		}
	}

	self->action = &entityWait;
}

static void entityWait()
{
	long onGround = self->flags & ON_GROUND;

	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

		if (self->thinkTime <= 0)
		{
			/* Randomly jump another way */

			setEntityAnimation(self, "JUMP");

			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}

			self->thinkTime = 30 + prand() % 30;

			self->dirY = -(8 + prand() % 2);

			if (isAtEdge(self) == TRUE)
			{
				self->face = self->face == LEFT ? RIGHT : LEFT;
			}

			self->dirX = 4 * (self->face == LEFT ? -1 : 1);
		}

		else
		{
			self->thinkTime--;

			if (self->endY == -1)
			{
				playSoundToMap("sound/enemy/frog/croak", -1, self->x, self->y, 0);

				self->endY = 0;
			}

			if (self->mental <= 0 && player.health > 0 && prand() % 45 == 0)
			{
				if (collision(self->x + (self->face == RIGHT ? self->w : -120), self->y, 120, self->h, player.x, player.y, player.w, player.h) == 1)
				{
					self->thinkTime = 30;

					setEntityAnimation(self, "ATTACK_1");

					self->action = &tongueAttackInit;
				}
			}
		}
	}

	else
	{
		if (self->dirX == 0)
		{
			self->face = self->face == LEFT ? RIGHT : LEFT;

			self->dirX = self->face == LEFT ? -4 : 4;
		}
	}

	checkToMap(self);

	if ((self->flags & ON_GROUND) && onGround == 0)
	{
		setEntityAnimation(self, "STAND");

		if (prand() % 3 == 0)
		{
			self->endY = -1;
		}
	}

	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;
		self->target->y = self->y + self->h / 2 - self->target->h / 2;

		self->mental--;

		if (self->mental <= 0)
		{
			self->target->flags &= ~NO_DRAW;

			setCustomAction(self->target, &invulnerable, 60, 0, 0);

			self->target->dirY = ITEM_JUMP_HEIGHT;

			self->target = NULL;
		}

		else
		{
			setCustomAction(self->target, &invulnerableNoFlash, 15, 0, 0);
		}
	}
}

static void tongueAttackInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &tongueAttack;
	}

	checkToMap(self);
}

static void tongueAttack()
{
	Entity *e;

	setEntityAnimation(self, "ATTACK_2");

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Frog Tongue");
	}

	loadProperties("enemy/frog_tongue", e);

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

	e->startX = e->x;
	e->startY = e->y;

	e->endX = e->startX + (e->face == LEFT ? -120 : 120);

	e->endY = getMapFloor(e->x, e->y);

	e->dirX = e->face == LEFT ? -e->speed : e->speed;

	e->action = &tongueMove;

	e->touch = &tongueTouch;

	e->draw = &drawTongue;

	e->die = &entityDieNoDrop;

	e->type = ENEMY;

	e->head = self;

	self->mental = -1;

	checkToMap(self);

	self->action = &tongueAttackWait;
}

static void tongueAttackWait()
{
	if (self->mental != -1)
	{
		setEntityAnimation(self, "STAND");

		self->action = &entityWait;
	}

	checkToMap(self);
}

static void tongueMove()
{
	if (self->head->health <= 0)
	{
		if (self->target != NULL)
		{
			self->target->flags &= ~NO_DRAW;

			setCustomAction(self->target, &invulnerable, 60, 0, 0);

			self->target->dirY = ITEM_JUMP_HEIGHT;

			self->target = NULL;
		}

		self->action = self->die;
	}

	if (self->dirX == 0 || (self->face == LEFT && self->x <= self->endX) || (self->face == RIGHT && self->x >= self->endX))
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->action = &tongueReturn;
	}

	checkToMap(self);
}

static void tongueReturn()
{
	self->x += self->dirX;

	if (self->head->health <= 0)
	{
		if (self->target != NULL)
		{
			self->target->flags &= ~NO_DRAW;

			setCustomAction(self->target, &invulnerable, 60, 0, 0);

			self->target->dirY = ITEM_JUMP_HEIGHT;

			self->target = NULL;
		}

		self->action = self->die;
	}

	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;
		self->target->y = self->y + self->h / 2 - self->target->h / 2;
	}

	if ((self->face == LEFT && self->x >= self->startX) || (self->face == RIGHT && self->x <= self->startX))
	{
		if (self->target != NULL)
		{
			if (self->target->type == PLAYER)
			{
				self->target->fallout();

				self->target = NULL;

				self->head->mental = 0;
			}

			else
			{
				self->head->target = self->target;

				self->head->target->flags |= NO_DRAW;

				self->head->mental = 60 * 60;

				self->target = NULL;

				setInfoBoxMessage(180, 255, 255, 255, _("Your %s has been stolen!"), self->head->target->objectiveName);

				STRNCPY(self->head->objectiveName, self->head->target->name, sizeof(self->head->objectiveName));

				addExitTrigger(self->head->target);
			}
		}

		else
		{
			self->head->mental = 0;
		}

		self->head->thinkTime = 60;

		self->inUse = FALSE;
	}
}

static void tongueTouch(Entity *other)
{
	if (other->health > 0 && !(other->flags & INVULNERABLE) && other->type == PLAYER && self->target == NULL)
	{
		self->x = other->x + other->w / 2 - self->w / 2;

		self->targetY = other->y;

		self->target = other;

		self->action = &grabPause;

		self->thinkTime = 15;

		self->dirX = 0;

		playSoundToMap("sound/boss/armour_boss/tongue_start", -1, self->x, self->y, 0);
	}
}

static void grabPause()
{
	Entity *e;

	self->thinkTime--;

	self->target->x = self->x - self->target->w / 2 + self->w / 2;

	self->target->y = self->targetY;

	if (self->thinkTime <= 0)
	{
		self->target = NULL;

		if (prand() % 2 == 0)
		{
			e = removePlayerShield();

			if (e == NULL)
			{
				e = removePlayerWeapon();
			}
		}

		else
		{
			e = removePlayerWeapon();

			if (e == NULL)
			{
				e = removePlayerShield();
			}
		}

		if (e != NULL)
		{
			setCustomAction(e, &invulnerableNoFlash, 180, 0, 0);

			self->target = e;
		}

		else
		{
			self->target = &player;

			self->head->touch = &frogTouch;
		}

		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->action = &tongueReturn;

		self->touch = NULL;

		if (self->target->health > 0)
		{
			setPlayerLocked(TRUE);

			setPlayerLocked(FALSE);
		}
	}
}

static void die()
{
	if (self->target != NULL)
	{
		self->target->flags &= ~NO_DRAW;

		setCustomAction(self->target, &invulnerable, 60, 0, 0);

		self->target->dirY = ITEM_JUMP_HEIGHT;
	}

	entityDie();
}

static int drawTongue()
{
	float startX;

	startX = self->x;

	/* Draw the tongue first */

	self->x = self->startX - (self->face == RIGHT ? 0 : 0);

	setEntityAnimation(self, "WALK");

	if (self->face == RIGHT)
	{
		while (self->x < startX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	else
	{
		while (self->x > startX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	/* Draw the tip */

	setEntityAnimation(self, "STAND");

	self->x = startX;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void addExitTrigger(Entity *e)
{
	char itemName[MAX_LINE_LENGTH];

	snprintf(itemName, MAX_LINE_LENGTH, "\"%s\" 1 UPDATE_EXIT \"FROG\"", e->objectiveName);

	addGlobalTriggerFromScript(itemName);
}

static void frogTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->health = 0;

		other->flags |= NO_DRAW;

		other->fallout();

		playSoundToMap("sound/enemy/whirlwind/suck", -1, self->x, self->y, 0);
	}
}

static void creditsMove()
{
	float dirX;
	long onGround = self->flags & ON_GROUND;

	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "JUMP");

			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}

			self->thinkTime = 30 + prand() % 30;

			self->dirY = -(8 + prand() % 2);

			self->dirX = 4;
		}

		else
		{
			self->thinkTime--;

			if (self->endY == -1)
			{
				playSoundToMap("sound/enemy/frog/croak", -1, self->x, self->y, 0);

				self->endY = 0;
			}
		}
	}

	dirX = self->dirX;

	checkToMap(self);

	if ((self->flags & ON_GROUND) && onGround == 0)
	{
		setEntityAnimation(self, "STAND");

		if (prand() % 3 == 0)
		{
			self->endY = -1;
		}
	}

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}
}
