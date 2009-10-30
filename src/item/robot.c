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
#include "../collisions.h"
#include "key_items.h"
#include "../map.h"
#include "../hud.h"
#include "../inventory.h"
#include "../event/script.h"
#include "../custom_actions.h"

extern Entity *self, player;

static void touch(Entity *);
static void activate(int);
static void processNextInstruction(void);
static void instructionMove(void);
static void finish(void);
static void returnMove(void);
static void wait(void);

Entity *addRobot(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Robot\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (self->active == FALSE && other->type == PLAYER)
	{
		setInfoBoxMessage(0, _("Press Action to interact"));
	}

	else if (self->health == 1 && self->dirX == 0 && self->dirY == 0)
	{
		if (strcmpignorecase(other->name, "item/robot_return"))
		{
			switch (other->health)
			{
				case 0:
					self->dirY = -self->speed;
				break;

				case 1:
					self->dirY = self->speed;
				break;

				case 2:
					self->dirX = -self->speed;
				break;

				case 3:
					self->dirX = self->speed;
				break;
			}
		}
	}
}

static void activate(int val)
{
	Entity *e = getCurrentInventoryItem();

	if (e == NULL || strcmpignorecase(e->name, "item/instruction_card") != 0)
	{
		runScript("instruction_card");
	}

	else
	{
		e = addEntity(*e, self->x, self->y);

		removeInventoryItem(e->objectiveName);

		self->target = e;

		self->action = &processNextInstruction;

		self->active = TRUE;

		self->mental = 0;

		self->health = 1;

		setCustomAction(&player, &helpless, 5, 0);

		centerMapOnEntity(self);
	}
}

static void processNextInstruction()
{
	char token;

	self->dirX = 0;
	self->dirY = 0;

	token = self->target->requires[self->mental];

	switch (token)
	{
		case 'u':
			self->dirY = -self->speed;
		break;

		case 'd':
			self->dirY = self->speed;
		break;

		case 'l':
			self->dirX = -self->speed;
		break;

		case 'r':
			self->dirX = self->speed;
		break;
	}

	if (self->dirX != 0 || self->dirY != 0)
	{
		self->action = &instructionMove;
	}

	else
	{
		self->thinkTime = 30;

		self->action = &finish;
	}

	self->mental++;
}

static void instructionMove()
{
	checkToMap(self);

	setCustomAction(&player, &helpless, 5, 0);

	if (self->dirX == 0 && self->dirY == 0)
	{
		processNextInstruction();
	}
}

static void finish()
{
	self->thinkTime--;

	setCustomAction(&player, &helpless, 5, 0);

	if (self->thinkTime <= 0)
	{
		self->mental = 0;

		self->action = &returnMove;
	}
}

static void returnMove()
{
	checkToMap(self);

	setCustomAction(&player, &helpless, 5, 0);

	if (self->dirX == 0 && self->dirY == 0)
	{
		centerMapOnEntity(&player);

		self->action = &wait;
	}
}
