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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../item/item.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self, player;

static void throwBooks(void);
static void bookLookForPlayer(void);
static void createBooks(void);
static void rotateAroundTarget(void);
static void bookAttackPlayer(void);
static void bookReactToBlock(void);
static void bookAttackEnd(void);
static void bookReturn(void);
static void bookDie(void);
static void takeDamage(Entity *, int);
static void stunned(void);
static void stunFinish(void);
static void lightKill(int);

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

		e->takeDamage = &entityTakeDamageNoFlinch;
	}

	else if (strcmpignorecase(name, "enemy/poltergiest_2") == 0)
	{
		e->action = &createBooks;

		e->activate = &lightKill;

		e->takeDamage = &takeDamage;
	}

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

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

			self->thinkTime = 180;
		}
	}
}

static void throwBooks()
{
	self->dirX = 0;

	if (self->mental <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			createBooks();
		}
	}
}

static void createBooks()
{
	int i;
	Entity *e;

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Poltergiest Book");
		}

		loadProperties("enemy/book", e);

		e->x = self->x;
		e->y = self->y;

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->thinkTime = i * 60;

		e->action = &rotateAroundTarget;
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
	y = 128;

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	radians = DEG_TO_RAD(self->thinkTime);

	self->x = (x * cos(radians) - y * sin(radians));
	self->y = (x * sin(radians) + y * cos(radians));

	self->x += self->head->x;
	self->y += self->head->y;

	self->x += (self->head->w - self->w) / 2;
	self->y += (self->head->h - self->h) / 2;

	if (self->head->action == &throwBooks)
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

			self->dirX *= 4;
			self->dirY *= 4;
		}
	}
}

static void bookAttackPlayer()
{
	float dirX, dirY;

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	checkToMap(self);

	if ((self->dirX == 0 && dirX != 0) || (self->dirY == 0 && dirY != 0))
	{
		self->action = &bookReturn;
	}
}

static void bookReactToBlock()
{
	if (player.face == LEFT)
	{
		self->x = player.x - self->w;
	}

	else
	{
		self->x = player.x + player.w;
	}

	self->dirX = player.face == LEFT ? -5 : 5;

	self->dirY = -6;

	self->action = &bookAttackEnd;

	self->thinkTime = 120;

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}
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
		self->action = &bookReturn;
	}

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
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

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	checkToMap(self);

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	if (fabs(startX - endX) <= fabs(self->dirX) && fabs(startY - endY) <= fabs(self->dirY))
	{
		self->action = &rotateAroundTarget;
	}
}

static void bookDie()
{
	self->target->mental--;

	entityDie();
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (self->health > 0)
		{
			self->health -= damage;

			if (self->health <= 0)
			{
				self->touch = NULL;

				self->action = &stunned;

				if (prand() % 3 == 0)
				{
					setInfoBoxMessage(60, _("Try hitting the light..."));
				}

				self->thinkTime = 300;
			}
		}
	}
}

static void stunned()
{
	self->flags &= ~FLY;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;

		self->touch = &entityTouch;

		self->action = &stunFinish;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}
}

static void stunFinish()
{
	self->flags |= FLY;

	self->dirY = -4;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &bookLookForPlayer;
	}
}

static void lightKill(int val)
{
	Entity *e;

	if (val == -1 && self->health <= 0)
	{
		entityDie();

		e = addPermanentItem("item/code_card", self->x + self->w / 2, self->y);

		e->x -= e->w / 2;

		e->dirY = ITEM_JUMP_HEIGHT;

		STRNCPY(e->objectiveName, self->requires, sizeof(e->objectiveName));
	}
}
