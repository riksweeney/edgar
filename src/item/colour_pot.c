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
#include "../graphics/animation.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void touch(Entity *);
static void mixColours(int);
static void collectColour(int);
static int draw(void);
static void drain(void);
static void refill(void);

Entity *addColourPot(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Colour Pot");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;

	e->draw = &draw;

	e->touch = &touch;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->startY = self->y + self->offsetY;
	self->endY = self->y + self->offsetY + 100;

	self->target = getEntityByObjectiveName(self->requires);

	if (self->target == NULL)
	{
		showErrorAndExit("Colour Pot cannot find target %s", self->requires);
	}

	self->activate = self->health == -1 ? &collectColour : &mixColours;

	self->action = &entityWait;
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
}

static void mixColours(int val)
{
	int currentColour = self->target->mental;

	if (currentColour == 0)
	{
		currentColour = self->mental;

		self->target->action = &refill;

		self->target->thinkTime = 100;
	}

	else
	{
		switch (currentColour)
		{
			case 1: /* Red */
				switch (self->mental)
				{
					case 1: /* Red */
						currentColour = 1; /* Red */
					break;

					case 2: /* Yellow */
						currentColour = 4; /* Orange */
					break;

					default: /* Blue */
						currentColour = 5; /* Purple */
					break;
				}
			break;

			case 2: /* Yellow */
				switch (self->mental)
				{
					case 1: /* Red */
						currentColour = 4; /* Orange */
					break;

					case 2: /* Yellow */
						currentColour = 2; /* Yellow */
					break;

					default: /* Blue */
						currentColour = 6; /* Green */
					break;
				}
			break;

			case 3: /* Blue */
				switch (self->mental)
				{
					case 1: /* Red */
						currentColour = 5; /* Purple */
					break;

					case 2: /* Yellow */
						currentColour = 6; /* Green */
					break;

					default: /* Blue */
						currentColour = 3; /* Blue */
					break;
				}
			break;

			default: /* Brown, probably */
				currentColour = 7; /* Brown */
			break;
		}
	}

	self->action = &drain;

	self->thinkTime = 0;

	self->activate = NULL;

	self->target->mental = currentColour;
}

static void collectColour(int val)
{
	if (self->mental == 0)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("The pot is empty"));

		return;
	}

	self->action = &drain;
}

static int draw()
{
	int r, g, b;

	if (self->mental != 0)
	{
		switch (self->mental)
		{
			case 1: /* Red */
				r = 204;
				g = 0;
				b = 0;
			break;

			case 2: /* Yellow */
				r = 204;
				g = 204;
				b = 0;
			break;

			case 3: /* Blue */
				r = 0;
				g = 0;
				b = 204;
			break;

			case 4: /* Orange */
				r = 204;
				g = 132;
				b = 0;
			break;

			case 5: /* Purple */
				r = 204;
				g = 0;
				b = 204;
			break;

			case 6: /* Green */
				r = 0;
				g = 204;
				b = 0;
			break;

			default: /* Brown */
				r = 139;
				g = 69;
				b = 19;
			break;
		}

		drawBoxToMap(self->x, self->startY, self->w, (self->endY - self->startY), r, g, b);
	}

	return drawLoopingAnimationToMap();
}

static void drain()
{
	char requiredColour, colour;
	Entity *temp;

	self->thinkTime += 3;

	if (self->thinkTime >= 100)
	{
		if (self->health == -1)
		{
			switch (self->mental)
			{
				case 1: /* Red */
					colour = 'R';
				break;

				case 2: /* Yellow */
					colour = 'Y';
				break;

				case 3: /* Blue */
					colour = 'B';
				break;

				case 4: /* Orange */
					colour = 'O';
				break;

				case 5: /* Purple */
					colour = 'P';
				break;

				case 6: /* Green */
					colour = 'G';
				break;

				default: /* Brown */
					colour = 'B';
				break;
			}

			requiredColour = self->target->requires[self->target->health];

			if (requiredColour == colour)
			{
				self->target->health++;

				temp = self;

				self = self->target;

				self->activate(-1);

				self = temp;

				if (self->target->health == self->target->mental)
				{
					self->touch = NULL;

					self->activate = NULL;

					playSoundToMap("sound/item/ping", -1, self->x, self->y, 0);
				}

				else
				{
					playSoundToMap("sound/item/charge_beep", -1, self->x, self->y, 0);
				}
			}

			else if (self->target->health < self->target->mental)
			{
				playSoundToMap("sound/item/buzzer", -1, self->x, self->y, 0);
			}

			self->mental = 0;

			self->action = &entityWait;
		}

		else
		{
			self->action = &refill;

			self->thinkTime = 100;
		}
	}

	self->startY = self->y + self->offsetY + self->thinkTime;

	checkToMap(self);
}

static void refill()
{
	self->thinkTime -= 3;

	if (self->thinkTime <= 0)
	{
		self->action = &entityWait;

		self->activate = self->health == -1 ? &collectColour : &mixColours;

		self->thinkTime = 0;
	}

	self->startY = self->y + self->offsetY + self->thinkTime;

	checkToMap(self);
}
