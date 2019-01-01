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
#include "../audio/music.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../medal.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void walk(void);
static void die(void);
static void entityWait(void);
static void riftAttackInit(void);
static void riftAttack(void);
static void riftAttackFinish(void);
static void changeWalkDirectionStart(void);
static void changeWalkDirection(void);
static void changeWalkDirectionFinish(void);
static void takeDamage(Entity *, int);
static void addRiftEnergy(int, int);
static void energyMoveToRift(void);
static void riftMove(void);
static void riftWait(void);
static void riftClose(void);
static void gibWait(void);
static void riftDestroyWait(void);
static void riftTouch(Entity *);
static void spinAttackInit(void);
static void spinAttack(void);
static void spinAttackEnd(void);
static void addSparkles(void);
static int draw(void);
static void resumeNormal(void);
static void breatheFireInit(void);
static void breatheFireWait(void);
static void castIce(void);
static void iceDrop(void);
static void init(void);
static void creditsMove(void);

Entity *addRampagingMasterTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Rampaging Master Tortoise");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &draw;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;
	e->resumeNormalFunction = &resumeNormal;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->mental = 0;

	self->startX = 0;

	self->action = &walk;
}

static void walk()
{
	if (self->maxThinkTime == 1)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			self->maxThinkTime = 1;

			playSoundToMap("sound/enemy/rampaging_master_tortoise/stomp", -1, self->x, self->y, 0);
		}

		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	if (self->maxThinkTime == 0 && (self->dirX == 0 || isAtEdge(self) == TRUE))
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	checkToMap(self);

	if (player.health > 0)
	{
		if (self->mental != -1 && prand() % 60 == 0)
		{
			self->dirX = 0;

			self->action = &riftAttackInit;

			self->thinkTime = 30;
		}

		else if (prand() % 60 == 0
			&& collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			switch (prand() % 3)
			{
				case 0:
					self->dirX = 0;

					self->thinkTime = 60;

					self->action = &spinAttackInit;
				break;

				case 1:
					self->dirX = 0;

					self->thinkTime = 15;

					self->action = &castIce;
				break;

				default:
					self->thinkTime = 15;

					self->action = &breatheFireInit;

					self->dirX = 0;
				break;
			}
		}
	}
}

static void riftAttackInit()
{
	self->thinkTime--;

	setEntityAnimation(self, "ATTACK_1");

	if (self->thinkTime <= 0)
	{
		self->action = &riftAttack;
	}

	checkToMap(self);
}

static void riftAttack()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Energy Rift");
	}

	loadProperties("enemy/energy_rift", e);

	e->action = &riftMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	if (self->face == LEFT)
	{
		e->x = self->x - e->w;
	}

	else
	{
		e->x = self->x + self->w;
	}

	e->thinkTime = 15;

	e->y = self->y;

	e->dirX = self->face == LEFT ? -e->speed : e->speed;

	e->head = self;

	checkToMap(self);

	self->mental = -1;

	self->action = &riftAttackFinish;

	self->thinkTime = 60;
}

static void riftAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &walk;
	}

	checkToMap(self);
}

static void die()
{
	Entity *e;

	playSoundToMap("sound/enemy/tortoise/tortoise_die", -1, self->x, self->y, 0);

	if (getInventoryItemByObjectiveName("Tortoise Shell") == NULL)
	{
		e = dropCollectableItem("item/tortoise_shell", self->x + self->w / 2, self->y, self->face);

		e->x -= e->w / 2;
	}

	addMedal("kill_rampaging");

	if (self->startX == -1)
	{
		fadeBossMusic();
	}

	entityDie();
}

static void takeDamage(Entity *other, int damage)
{
	entityTakeDamageNoFlinch(other, damage);

	if ((prand() % 2 == 0) && self->face == other->face && self->health > 0 && self->dirX != 0 && self->speed == self->originalSpeed)
	{
		self->dirX = 0;

		self->action = &changeWalkDirectionStart;
	}
}

static void entityWait()
{
	checkToMap(self);
}

static void changeWalkDirectionStart()
{
	setEntityAnimation(self, "CUSTOM_1");

	self->action = &entityWait;

	self->animationCallback = &changeWalkDirection;

	self->thinkTime = 60;

	checkToMap(self);
}

static void changeWalkDirection()
{
	self->thinkTime--;

	self->action = &changeWalkDirection;

	self->creditsAction = &changeWalkDirection;

	setEntityAnimation(self, "CUSTOM_2");

	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->frameSpeed = -1;

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &changeWalkDirectionFinish;

		self->action = &entityWait;

		self->creditsAction = &entityWait;
	}

	checkToMap(self);
}

static void changeWalkDirectionFinish()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, "STAND");

	self->action = &walk;

	self->creditsAction = &creditsMove;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;

	checkToMap(self);
}

static void riftMove()
{
	self->dirX *= 0.95;

	if (fabs(self->dirX) <= 0.5)
	{
		self->dirX = 0;

		if (self->health < 2)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->health++;

				setEntityAnimation(self, self->health == 1 ? "WALK" : "JUMP");

				self->thinkTime = 15;
			}
		}

		else
		{
			self->health = playSoundToMap("sound/item/rift", -1, self->x, self->y, -1);

			self->thinkTime = 300;

			self->touch = &riftTouch;

			self->action = &riftWait;
		}
	}

	checkToMap(self);
}

static void riftWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		stopSound(self->health);

		self->action = &riftClose;

		self->thinkTime = 20;
	}

	else
	{
		if (collision(self->x - self->mental, self->y - self->mental, self->mental * 2, self->mental * 2, player.x, player.y, player.w, player.h) == 1)
		{
			setCustomAction(&player, &attract, 5, 0, (player.x < (self->x + self->w / 2) ? 2 : -2));
		}

		if (prand() % 3 == 0)
		{
			addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
		}
	}
}

static void riftClose()
{
	self->thinkTime--;

	setEntityAnimation(self, self->thinkTime > 10 ? "WALK" : "STAND");

	if (self->thinkTime <= 0)
	{
		self->head->mental = 0;

		self->inUse = FALSE;
	}
}

static void addRiftEnergy(int x, int y)
{
	Entity *e;

	e = addBasicDecoration(x, y, "decoration/rift_energy");

	e->x += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);
	e->y += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);

	x -= e->w / 2;
	y -= e->h / 2;

	e->targetX = x;
	e->targetY = y;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->dirX *= 8;
	e->dirY *= 8;

	e->action = &energyMoveToRift;
}

static void energyMoveToRift()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (atTarget())
	{
		self->inUse = FALSE;
	}
}

static void riftTouch(Entity *other)
{
	int i;
	Entity *temp, *e;
	EntityList *list, *l;

	i = 0;

	if (other->type == PLAYER && player.health > 0)
	{
		temp = self;

		self = other;

		list = playerGib();

		for (l=list->next;l!=NULL;l=l->next)
		{
			e = l->entity;

			e->head = temp;

			e->x = temp->x + temp->w / 2 - e->w / 2;
			e->y = temp->y + temp->h / 2 - e->h / 2;

			e->startX = e->x;
			e->startY = e->y;

			e->mental = 0;

			e->health = prand() % 360;

			e->dirX = 1;

			e->action = &gibWait;

			e->thinkTime = 180;

			i++;
		}

		freeEntityList(list);

		self = temp;

		self->action = &riftDestroyWait;

		self->mental = i;
	}
}

static void riftDestroyWait()
{
	checkToMap(self);

	if (self->mental <= 0)
	{
		stopSound(self->health);

		self->thinkTime = 20;

		self->action = &riftClose;
	}

	if (prand() % 3 == 0)
	{
		addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
	}
}

static void gibWait()
{
	float radians;

	if (self->dirX == -1)
	{
		self->mental -= 2;

		if (self->mental <= 0)
		{
			self->head->mental--;

			self->inUse = FALSE;
		}
	}

	else
	{
		self->mental += 2;

		if (self->mental >= 160)
		{
			self->mental = 160;

			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->dirX = -1;
			}
		}
	}

	self->health += 8;

	radians = DEG_TO_RAD(self->health);

	self->x = (0 * cos(radians) - self->mental * sin(radians));
	self->y = (0 * sin(radians) + self->mental * cos(radians));

	self->x += self->startX;
	self->y += self->startY;
}

static void spinAttackInit()
{
	setEntityAnimation(self, "ATTACK_2");

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		self->action = &spinAttack;

		self->endX = prand() % 3;

		self->speed = self->speed * 8;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	checkToMap(self);
}

static void spinAttack()
{
	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self))
	{
		self->endX--;

		if (self->endX <= 0)
		{
			if (self->dirX == 0)
			{
				self->dirX = self->face == LEFT ? 3 : -3;
			}

			else
			{
				self->dirX = self->dirX < 0 ? 3 : -3;
			}

			self->dirY = -6;

			self->action = &spinAttackEnd;
		}

		else
		{
			self->face = self->face == LEFT ? RIGHT : LEFT;

			self->dirX = self->face == LEFT ? -self->speed : self->speed;
		}

		playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);
	}
}

static void spinAttackEnd()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		facePlayer();

		setEntityAnimation(self, "STAND");

		self->speed = self->originalSpeed;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &walk;

		self->thinkTime = 60;
	}
}

static void breatheFireInit()
{
	Entity *e;

	setEntityAnimation(self, "ATTACK_1");

	e = addProjectile("enemy/fireball", self, 0, 0, (self->face == LEFT ? -6 : 6), 0);

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - self->offsetX;
	}

	else
	{
		e->x = self->x + self->offsetX;
	}

	e->y = self->y + self->offsetY;

	e->flags |= FLY;

	playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);

	self->thinkTime = 30;

	self->action = &breatheFireWait;

	checkToMap(self);
}

static void breatheFireWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 300 + prand() % 180;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->action = &walk;
	}

	checkToMap(self);
}

static void castIce()
{
	Entity *e;

	self->thinkTime--;

	setEntityAnimation(self, "ATTACK_1");

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice Spike");
		}

		loadProperties("enemy/ice_spike", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = player.x + player.w / 2;
		e->targetY = getMapCeiling(e->targetX, self->y);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		e->action = &iceDrop;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 30;

		e->flags |= FLY|DO_NOT_PERSIST;

		self->thinkTime = 0;

		setEntityAnimation(self, "STAND");

		self->action = &walk;
	}

	checkToMap(self);
}

static void iceDrop()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

		for (i=0;i<8;i++)
		{
			e = addTemporaryItem("misc/ice_spike_piece", self->x, self->y, RIGHT, 0, 0);

			e->x = self->x + self->w / 2;
			e->x -= e->w / 2;

			e->y = self->y + self->h / 2;
			e->y -= e->h / 2;

			e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT * 2 + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);

			e->touch = NULL;
		}

		self->inUse = FALSE;
	}
}

static void addSparkles()
{
	Entity *e;

	e = addBasicDecoration(self->x, self->y, "decoration/particle");

	if (e != NULL)
	{
		e->x += prand() % self->box.w;
		e->y += prand() % self->box.h;

		e->thinkTime = 5 + prand() % 30;

		setEntityAnimationByID(e, prand() % 5);

		e->dirX = 0;
		e->dirY = -2;
	}
}

static int draw()
{
	int offsetX = self->offsetX;
	int drawn = drawLoopingAnimationToMap();

	if (drawn == TRUE)
	{
		if (game.status == IN_GAME && offsetX == 0 && self->offsetX != offsetX)
		{
			shakeScreen(LIGHT, 5);
		}

		addSparkles();
	}

	return drawn;
}

static void resumeNormal()
{
	self->face = player.x < self->x ? RIGHT : LEFT;

	self->action = &changeWalkDirection;

	self->creditsAction = &changeWalkDirection;
}

static void creditsMove()
{
	float dirX;

	if (self->maxThinkTime == 1)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			self->maxThinkTime = 1;

			playSoundToMap("sound/enemy/rampaging_master_tortoise/stomp", -1, self->x, self->y, 0);
		}

		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}
}
