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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Input input;

static void entityWait(void);
static void activate(int);
static void init(void);
static void pegWait(void);
static void activatePeg(void);
static void addCursor(void);
static void cursorWait(void);
static void generateSolution(void);
static int checkSolution(void);
static void pegInit(void);

Entity *addMastermindDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mastermind Display");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addMastermindPeg(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mastermind Peg");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &pegInit;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addMastermindScore(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mastermind Score");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &pegInit;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);

	if (self->mental == -1)
	{
		generateSolution();

		self->mental = 0;
	}
}

static void activate(int val)
{
	if (val == 1)
	{
		self->endX -= TILE_SIZE;

		if (self->endX < self->startX)
		{
			self->endX = self->startX;
		}

		else
		{
			playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
		}
	}

	else if (val == 2)
	{
		self->endX += TILE_SIZE;

		if (self->endX >= self->startX + self->w)
		{
			self->endX = self->startX + self->w - TILE_SIZE;
		}

		else
		{
			playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
		}
	}

	else if (val == 3)
	{
		playSoundToMap("sound/common/click", -1, self->x, self->y, 0);

		activatePeg();
	}
}

static void init()
{
	int i, x, y, col;
	Entity *e, *prev;

	prev = self;

	x = self->x;
	y = self->y + self->h;

	col = 1;

	if (self->mental != 2)
	{
		for (i=0;i<50;i++)
		{
			/* 4 pegs per row, plus 1 score tile and 10 rows */

			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Mastermind Peg");
			}

			if (col == 5)
			{
				loadProperties("item/mastermind_score", e);

				col = 0;

				e->mental = 1;
			}

			else
			{
				loadProperties("item/mastermind_peg", e);

				e->mental = 0;
			}

			setEntityAnimation(e, "STAND");

			if (i == 0)
			{
				y -= TILE_SIZE;
			}

			if (i != 0 && i % 5 == 0)
			{
				x = self->x;
				y -= TILE_SIZE;
			}

			else if (i != 0)
			{
				x += TILE_SIZE;
			}

			e->face = RIGHT;

			e->x = x;
			e->y = y;

			e->action = &pegWait;

			e->draw = &drawLoopingAnimationToMap;

			prev->target = e;

			prev = e;

			e->target = NULL;

			col++;
		}

		generateSolution();

		addCursor();
	}

	self->action = &entityWait;
}

static void pegWait()
{

}

static void activatePeg()
{
	int val;
	Entity *e;

	e = self->target;

	while (e != NULL)
	{
		if (e->x == self->endX && e->y == self->endY)
		{
			if (e->mental == 0)
			{
				e->health++;

				if (e->health > self->health)
				{
					e->health = 1;
				}

				setEntityAnimationByID(e, e->health);
			}

			else
			{
				val = checkSolution();

				if (val == TRUE)
				{
					self->mental = 2;

					e = self->target;

					while (e != NULL)
					{
						e->flags &= ~DO_NOT_PERSIST;

						e = e->target;
					}
				}

				else if (val == FALSE)
				{
					self->endY -= TILE_SIZE;
					self->endX = self->x;

					if (self->endY < self->y)
					{
						self->endY = self->y;

						self->mental = 1;
					}
				}
			}

			break;
		}

		e = e->target;
	}
}

static void addCursor()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mastermind Cursor");
	}

	loadProperties("item/jigsaw_puzzle_cursor", e);

	e->x = self->endX;
	e->y = self->endY;

	e->action = &cursorWait;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void cursorWait()
{
	self->x = self->head->endX;
	self->y = self->head->endY;
}

static void generateSolution()
{
	int i;
	char c[2];
	Entity *e;

	for (i=0;i<4;i++)
	{
		snprintf(c, 2, "%ld", 1 + prand() % self->health);

		self->requires[i] = c[0];
	}

	self->requires[i] = '\0';

	e = self->target;

	while (e != NULL)
	{
		e->health = 0;

		setEntityAnimationByID(e, e->health);

		e = e->target;
	}

	self->endX = self->x;
	self->endY = self->y + self->h - TILE_SIZE;
}

static int checkSolution()
{
	int i, total;
	char solution[6], c[2];
	Entity *e;

	e = self->target;

	while (e != NULL)
	{
		if (e->y == self->endY && e->x == self->x)
		{
			break;
		}

		e = e->target;
	}

	if (e == NULL)
	{
		showErrorAndExit("Could not find starting peg for row");
	}

	for (i=0;i<4;i++)
	{
		if (e->health == 0)
		{
			setInfoBoxMessage(180, 255, 255, 255, _("Select a colour for every peg in the row"));

			return -1;
		}

		snprintf(c, 2, "%d", e->health);

		solution[i] = c[0];

		e = e->target;
	}

	solution[i] = '\0';

	total = 0;

	for (i=0;i<4;i++)
	{
		if (solution[i] == self->requires[i])
		{
			total++;
		}
	}

	e->health = total + 1;

	setEntityAnimationByID(e, e->health);

	return strcmpignorecase(solution, self->requires) == 0 ? TRUE : FALSE;
}

static void pegInit()
{
	setEntityAnimationByID(self, self->health);

	self->mental = 0;

	self->action = &entityWait;

	self->flags &= ~DO_NOT_PERSIST;
}
