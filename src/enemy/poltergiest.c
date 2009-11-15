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
#include "../player.h"

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
static void hover(void);
static void die(void);
static void recreateBooks(void);

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

		e->activate = &lightKill;

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
	float dirX, dirY;

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	dirX = self->dirX;
	dirY = self->dirY;

	checkToMap(self);

	if ((self->dirX == 0 && dirX != 0) || (self->dirY == 0 && dirY != 0))
	{
		self->action = &bookReturn;
	}

	else if (self->head->health <= 0)
	{
		entityDie();
	}
}

static void bookReactToBlock()
{
	self->flags &= ~FLY;

	self->dirX = self->x < player.x ? -5 : 5;

	self->dirY = -5;

	self->action = &bookAttackEnd;

	self->thinkTime = 120;

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
	if (val == -1 && self->health <= 0)
	{
		self->die();
	}
}

static void die()
{
	Entity *e;

	e = addPermanentItem("item/code_card", self->x + self->w / 2, self->y);

	e->x -= e->w / 2;

	e->dirY = ITEM_JUMP_HEIGHT;

	STRNCPY(e->objectiveName, self->objectiveName, sizeof(e->objectiveName));

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
