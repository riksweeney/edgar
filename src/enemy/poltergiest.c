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
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../item/item.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void throwBooks(void);
static void bookLookForPlayer(void);
static void createBooks(void);
static void rotateAroundTarget(void);
static void bookAttackPlayer(void);
static void bookReactToBlock(Entity *other);
static void bookAttackEnd(void);
static void bookReturn(void);
static void bookDie(void);
static void takeDamage(Entity *, int);
static void retreatToSkull(void);
static void recharge(void);
static void hover(void);
static void die(void);
static void recreateBooks(void);
static void skullWait(void);
static void init(void);

Entity *addPoltergiest(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Poltergiest");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	if (strcmpignorecase(name, "enemy/poltergiest_1") == 0)
	{
		e->action = &createBooks;
	}

	else if (strcmpignorecase(name, "enemy/poltergiest_2") == 0)
	{
		e->action = &createBooks;

		e->takeDamage = &entityTakeDamageNoFlinch;
	}

	else if (strcmpignorecase(name, "enemy/poltergiest_3") == 0)
	{
		e->action = &createBooks;

		e->takeDamage = &takeDamage;
	}

	else if (strcmpignorecase(name, "enemy/poltergiest_4") == 0)
	{
		e->action = &init;

		e->takeDamage = &takeDamage;
	}

	else
	{
		showErrorAndExit("Poltergiest name not defined correctly: %s", name);
	}

	e->die = &die;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void bookLookForPlayer()
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

	if (player.health > 0 && prand() % 5 == 0)
	{
		self->thinkTime = 0;

		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, 200, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &throwBooks;

			self->thinkTime = 300;
		}
	}

	hover();
}

static void throwBooks()
{
	float dirX;

	if (getDistanceFromPlayer(self) > SCREEN_WIDTH)
	{
		self->action = &bookLookForPlayer;
	}

	else
	{
		if (self->mental <= 0)
		{
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

			if (self->thinkTime <= 0)
			{
				self->thinkTime = 90;

				self->action = &recreateBooks;
			}
		}

		else
		{
			facePlayer();

			self->dirX = 0;
		}
	}

	hover();
}

static void recreateBooks()
{
	self->dirX = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &createBooks;
	}
}

static void createBooks()
{
	int i, j;
	Entity *e;

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Poltergiest Book");
		}

		j = prand() % 4;

		switch (j)
		{
			case 0:
				loadProperties("enemy/red_book", e);
			break;

			case 1:
				loadProperties("enemy/green_book", e);
			break;

			case 2:
				loadProperties("enemy/yellow_book", e);
			break;

			default:
				loadProperties("enemy/blue_book", e);
			break;
		}

		e->x = self->x;
		e->y = self->y;

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->thinkTime = i * 60;

		e->startX = e->thinkTime;

		e->flags |= DO_NOT_PERSIST;

		e->action = &rotateAroundTarget;
		e->pain = &enemyPain;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->die = &bookDie;
		e->takeDamage = &entityTakeDamageNoFlinch;

		e->mental = 60 + prand() % 180;

		e->head = self;
	}

	self->mental = 6;

	self->action = &bookLookForPlayer;
}

static void rotateAroundTarget()
{
	float x, y, radians;
	float startX, startY, endX, endY;

	x = 0;
	y = 64;

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	radians = DEG_TO_RAD(self->startX);

	self->x = (x * cos(radians) - y * sin(radians));
	self->y = (x * sin(radians) + y * cos(radians));

	self->x += self->head->x;
	self->y += self->head->y;

	self->x += (self->head->w - self->w) / 2;
	self->y += (self->head->h - self->h) / 2;

	if (self->head->action == &throwBooks && player.health > 0)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			self->action = &bookAttackPlayer;

			self->reactToBlock = &bookReactToBlock;

			startX = self->x + self->w / 2;
			startY = self->y + self->h / 2;

			endX = player.x + player.w / 2;
			endY = player.y + player.h / 2;

			calculatePath(startX, startY, endX, endY, &self->dirX, &self->dirY);

			self->dirX *= 8;
			self->dirY *= 8;
		}
	}

	else if (self->head->health <= 0)
	{
		entityDie();
	}
}

static void bookAttackPlayer()
{
	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	checkToMap(self);

	if (self->dirX == 0 || (self->flags & ON_GROUND))
	{
		self->flags &= ~FLY;

		self->dirX = self->x < player.x ? -5 : 5;

		self->dirY = -5;

		self->action = &bookAttackEnd;
	}

	else if (self->head->health <= 0)
	{
		entityDie();
	}
}

static void bookReactToBlock(Entity *other)
{
	self->thinkTime = 120;

	self->dirX = 0;
}

static void bookAttackEnd()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &bookReturn;
	}

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	if (self->head->health <= 0)
	{
		entityDie();
	}
}

static void bookReturn()
{
	float startX, startY, endX, endY;

	startX = self->x + self->w / 2;
	startY = self->y + self->h / 2;

	endX = self->head->x + self->head->w / 2;
	endY = self->head->y + self->head->h / 2;

	calculatePath(startX, startY, endX, endY, &self->dirX, &self->dirY);

	self->dirX *= 4;
	self->dirY *= 4;

	checkToMap(self);

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	/* If a book gets completely stuck in the scenery */

	if (self->dirX == 0 && self->dirY == 0)
	{
		self->x = endX;
		self->y = endY;

		startX = 0;
		startY = 0;

		endX = 0;
		endY = 0;
	}

	if (fabs(startX - endX) <= fabs(self->dirX) && fabs(startY - endY) <= fabs(self->dirY))
	{
		self->mental = 60 + prand() % 180;

		self->action = &rotateAroundTarget;
	}

	if (self->head->health <= 0)
	{
		entityDie();
	}
}

static void bookDie()
{
	self->head->mental--;

	if (strcmpignorecase(self->head->name, "enemy/poltergiest_1") == 0)
	{
		if (self->head->mental <= 0)
		{
			self->head->action = self->head->die;
		}
	}

	entityDie();
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (self->health > 0)
		{
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
		}

		else
		{
			self->action = &retreatToSkull;

			self->thinkTime = 300;
		}
	}
}

static void retreatToSkull()
{
	Entity *e;

	e = getEntityByObjectiveName(self->requires);

	if (e == NULL)
	{
		die();
	}

	else
	{
		self->target = e;

		self->targetX = e->x + e->w / 2 - self->w / 2;
		self->targetY = e->y + e->h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/teleport", -1, self->x, self->y, 0);

		self->thinkTime = 300;

		self->target->maxThinkTime++;

		if (self->target->mental >= 1)
		{
			self->target->mental = 1;

			self->target->maxThinkTime = 0;
		}

		if (self->target->maxThinkTime > 1)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Try hitting the skull..."));
		}

		self->action = strcmpignorecase(self->name, "enemy/poltergiest_3") == 0 ? &recharge : &skullWait;
	}
}

static void skullWait()
{
	Entity *temp;

	temp = self;

	self = temp->target;

	self->activate(1);

	self = temp;

	self->flags |= NO_DRAW;

	self->touch = NULL;

	if (self->target->health <= 0)
	{
		temp = addPermanentItem("item/code_card", self->target->x + self->target->w / 2, self->target->y);

		temp->x -= temp->w / 2;

		temp->dirY = ITEM_JUMP_HEIGHT;

		STRNCPY(temp->objectiveName, self->objectiveName, sizeof(temp->objectiveName));

		self->inUse = FALSE;
	}
}

static void recharge()
{
	Entity *temp;

	temp = self;

	self = temp->target;

	self->activate(1);

	self = temp;

	self->flags |= NO_DRAW;

	self->touch = NULL;

	self->thinkTime--;

	if (self->thinkTime <= 0 || self->target->health <= 0)
	{
		self->targetX = self->endX;
		self->targetY = self->endY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/teleport", -1, self->x, self->y, 0);

		self->health = self->maxHealth;

		self->action = &createBooks;

		self->touch = &entityTouch;

		temp = self;

		self = temp->target;

		self->activate(0);

		self = temp;
	}
}

static void die()
{
	Entity *e;

	e = addPermanentItem("item/code_card", self->x + self->w / 2, self->y);

	e->x -= e->w / 2;

	e->dirY = ITEM_JUMP_HEIGHT;

	STRNCPY(e->objectiveName, self->objectiveName, sizeof(e->objectiveName));

	increaseKillCount();

	entityDie();
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

static void init()
{
	if (self->health > 0)
	{
		self->action = &createBooks;
	}

	else
	{
		self->action = &retreatToSkull;

		self->thinkTime = 300;
	}
}
