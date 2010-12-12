/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../graphics/decoration.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../hud.h"
#include "../inventory.h"
#include "../custom_actions.h"
#include "../event/trigger.h"
#include "../graphics/graphics.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void activate(int);

Entity *addColourDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Colour Display");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;

	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	char col;
	int i, colour, prev;

	if (strlen(self->requires) == 0)
	{
		prev = -1;

		for (i=0;i<self->mental;i++)
		{
			colour = 1 + (prand() % 6);

			while (colour == prev)
			{
				colour = 1 + (prand() % 6);
			}

			switch (colour)
			{
				case 1: /* Red */
					col = 'R';
				break;

				case 2: /* Yellow */
					col = 'Y';
				break;

				case 3: /* Blue */
					col = 'B';
				break;

				case 4: /* Orange */
					col = 'O';
				break;

				case 5: /* Purple */
					col = 'P';
				break;

				default: /* Green */
					col = 'G';
				break;
			}

			self->requires[i] = col;

			prev = colour;
		}

		self->requires[i] = '\0';
	}

	activate(-1);

	self->action = &entityWait;
}

static void entityWait()
{
	checkToMap(self);
}

static void activate(int val)
{
	char col;

	if (val == -1)
	{
		col = self->requires[self->health];

		switch (col)
		{
			case 'R': /* Red */
				setEntityAnimation(self, 0);
			break;

			case 'Y': /* Yellow */
				setEntityAnimation(self, 1);
			break;

			case 'B': /* Blue */
				setEntityAnimation(self, 2);
			break;

			case 'O': /* Orange */
				setEntityAnimation(self, 3);
			break;

			case 'P': /* Purple */
				setEntityAnimation(self, 4);
			break;

			case 'G': /* Green */
				setEntityAnimation(self, 5);
			break;

			default: /* Off */
				setEntityAnimation(self, 6);
			break;
		}

		if (self->health == self->mental)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);
		}
	}
}
