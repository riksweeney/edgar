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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../player.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../hud.h"
#include "../event/script.h"

extern Entity *self;
extern Input input;

static void touch(Entity *);
static void entityWait(void);
static void activate(int);
static void generateInputCode(void);
static void readInputCode(void);

Entity *addSafe(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Safe");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;
	
	e->touch = &touch;
	
	e->activate = &activate;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	
}

static void touch(Entity *other)
{
	setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
}

static void activate(int val)
{
	generateInputCode();
	
	self->action = &readInputCode;
	
	self->objectiveName[0] = '\0';

	self->touch = NULL;

	self->activate = NULL;

	setPlayerLocked(TRUE);
}

static void generateInputCode()
{
	Entity *e;
	
	self->mental = 0;
	
	self->health = 0;
	
	if (strlen(self->requires) == 0)
	{
		e = getInventoryItem("Safe Combination");
		
		if (e == NULL)
		{
			runScript("no_combination");
		}
		
		else
		{
			STRNCPY(self->requires, e->requires, sizeof(self->requires));
		}
	}
}

static void readInputCode()
{
	int val;
	char code[MAX_VALUE_LENGTH];

	if (input.left == 1)
	{
		input.left = 0;

		val = -1;
	}

	else if (input.right == 1)
	{
		input.right = 0;

		val = 1;
	}
	
	else if (input.attack == 1 || input.block == 1)
	{
		input.attack = 0;
		input.block = 0;

		val = 5;
	}
	
	else
	{
		val = 0;
	}

	if (val != 0)
	{
		if (val == 1 || val == -1)
		{
			if (self->health != val)
			{
				if (self->health != 0)
				{
					snprintf(code, sizeof(code), "%s%d%s", self->objectiveName, abs(self->mental), self->health == -1 ? "L" : "R");
					
					STRNCPY(self->objectiveName, code, sizeof(self->objectiveName));
					
					self->mental = 0;
					
					printf("%s\n", self->objectiveName);
				}
				
				self->health = val;
			}
			
			self->mental += val;
		}
		
		if (val == 5)
		{
			snprintf(code, sizeof(code), "%s%d%s", self->objectiveName, abs(self->mental), self->health == -1 ? "L" : "R");
			
			STRNCPY(self->objectiveName, code, sizeof(self->objectiveName));
			
			printf("%s == %s\n", self->objectiveName, self->requires);
			
			if (strcmpignorecase(self->objectiveName, self->requires) == 0)
			{
				printf("Complete\n");
			}
			
			self->action = &entityWait;

			self->activate = &activate;

			self->touch = &touch;

			setPlayerLocked(FALSE);
		}
	}
}
