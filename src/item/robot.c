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

#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/script.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "key_items.h"

extern Entity *self, player;

static void touch(Entity *);
static void activate(int);
static void processNextInstruction(void);
static void instructionMove(void);
static void finish(void);
static void returnMove(void);
static void entityWait(void);
static void init(void);

Entity *addRobot(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Robot");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->health == 2)
	{
		self->touch = NULL;

		self->activate = NULL;
	}

	self->action = &entityWait;
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	float dirX, dirY;

	if (self->active == FALSE && other->type == PLAYER)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}

	else if (self->health == 1 && fabs(self->x - other->x) <= fabs(self->dirX) && fabs(self->y - other->y) <= fabs(self->dirY))
	{
		if (strcmpignorecase(other->name, "item/robot_direction") == 0)
		{
			dirY = dirX = 0;

			switch (other->health)
			{
				case 0:
					dirY = -self->speed;

					setEntityAnimation(self, "JUMP");
				break;

				case 1:
					dirY = self->speed;

					setEntityAnimation(self, "JUMP");
				break;

				case 2:
					dirX = -self->speed;

					setEntityAnimation(self, "WALK");

					self->face = LEFT;
				break;

				default:
					dirX = self->speed;

					setEntityAnimation(self, "WALK");

					self->face = RIGHT;
				break;
			}

			if (dirY != self->dirY || dirX != self->dirX)
			{
				self->x = other->x;
				self->y = other->y;
			}

			self->dirY = dirY;
			self->dirX = dirX;
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

	else if (strlen(e->requires) == 0)
	{
		runScript("no_instructions");
	}

	else
	{
		runScript("robot_start");

		e = addEntity(*e, self->x, self->y);

		e->touch = NULL;

		e->flags |= NO_DRAW;

		removeInventoryItemByObjectiveName(e->objectiveName);

		self->target = e;

		self->action = &processNextInstruction;

		self->active = TRUE;

		self->mental = 0;

		setPlayerLocked(TRUE);

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
			setEntityAnimation(self, "JUMP");
		break;

		case 'd':
			self->dirY = self->speed;
			setEntityAnimation(self, "JUMP");
		break;

		case 'l':
			self->dirX = -self->speed;

			setEntityAnimation(self, "WALK");

			self->face = LEFT;
		break;

		case 'r':
			self->dirX = self->speed;

			setEntityAnimation(self, "WALK");

			self->face = RIGHT;
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

	setCustomAction(&player, &helpless, 5, 0, 0);

	if (self->dirX == 0 && self->dirY == 0)
	{
		processNextInstruction();
	}

	else if (self->x == self->endX && self->y == self->endY)
	{
		runScript("robot_end");

		self->dirX = 0;
		self->dirY = 0;

		self->action = &entityWait;

		setEntityAnimation(self, "STAND");

		activateEntitiesWithRequiredName(self->objectiveName, TRUE);

		centerMapOnEntity(&player);

		setPlayerLocked(FALSE);

		self->health = 2;

		self->target->inUse = FALSE;

		self->target = NULL;

		self->touch = NULL;

		self->activate = NULL;
	}
}

static void finish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health = 1;

		self->mental = 0;

		self->action = &returnMove;
	}
}

static void returnMove()
{
	checkToMap(self);

	if (self->x == self->startX && self->y == self->startY)
	{
		runScript("robot_end");

		centerMapOnEntity(&player);

		self->action = &entityWait;

		self->dirX = 0;
		self->dirY = 0;

		self->active = FALSE;

		setPlayerLocked(FALSE);

		self->health = 0;

		self->action = &entityWait;

		self->target->flags &= ~NO_DRAW;

		self->target->x = self->x;

		self->target->y = self->y;

		self->target->dirY = ITEM_JUMP_HEIGHT;

		self->target->touch = &keyItemTouch;

		self->target = NULL;

		setEntityAnimation(self, "STAND");
	}
}
