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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../player.h"
#include "../hud.h"
#include "../inventory.h"

extern Entity *self;

static void activate(int);
static void wait(void);
static void init(void);
static void randomize(void);

Entity *addSlidingPuzzleDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Sliding Puzzle Display");
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

static void wait()
{
	checkToMap(self);
}

static void activate(int val)
{
	int i, x, y, dirX, dirY, found;
	Entity *e;
	
	x = self->targetX;
	y = self->targetY;
	
	dirY = 0;
	dirX = 0;
	
	switch (val)
	{
		case 1:
			dirY = -self->target->h;
		break;
		
		case 2:
			dirY = self->target->h;
		break;
		
		case 3:
			dirX = -self->target->w;
		break;
		
		default:
			dirX = self->target->w;
		break;
	}
	
	found = FALSE;
	
	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->targetX + dirX == self->targetX && e->targetY + dirY == self->targetY)
		{
			e->x += dirX;
			e->targetX += dirX;
			
			e->y += dirY;
			e->targetY += dirY;
			
			self->targetX -= dirX;
			self->targetY -= dirY;
			
			playSoundToMap("sound/common/click.ogg", -1, self->x, self->y, 0);
			
			found = TRUE;
			
			break;
		}
	}
	
	if (found == TRUE)
	{
		x = self->x;
		y = self->y;
		
		i = 0;
		
		for (e=self->target;e!=NULL;e=e->target)
		{
			if (i != 0 && (i % 3 == 0))
			{
				x = self->x;
				
				y += e->h;
			}
			
			if (!(e->targetX == x && e->targetY == y))
			{
				found = FALSE;
				
				break;
			}
			
			i++;
			
			x += e->w;
		}
	}
	
	if (found == TRUE)
	{
		self->mental = 1;
	}
}

static void init()
{
	int i;
	Entity *e, *prev;
	
	prev = self;
	
	for (i=0;i<8;i++)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Sliding Puzzle Block");
		}
		
		loadProperties("item/sliding_puzzle_block", e);
		
		setEntityAnimation(e, i);
		
		e->health = i + 1;
		
		e->action = &wait;
		
		e->draw = &drawLoopingAnimationToMap;
		
		e->face = RIGHT;
		
		prev->target = e;
		
		prev = e;
	}
	
	if (self->mental == 0)
	{
		randomize();
	}
	
	self->action = &wait;
}

static void randomize()
{
	int i, j, tiles[9], temp, boardWidth, tileWidth;
	Entity *e;
	
	tileWidth = self->target->w;
	boardWidth = self->w;
	
	for (i=0;i<9;i++)
	{
		tiles[i] = i * tileWidth;
	}
	
	for (i=0;i<9;i++)
	{
		j = prand() % 9;
		
		temp = tiles[i];
		
		tiles[i] = tiles[j];
		
		tiles[j] = temp;
		
		if (i == 8)
		{
			self->targetX = self->x + (tiles[i] % boardWidth);
			self->targetY = self->y + (tiles[i] / boardWidth) * tileWidth;
		}
	}
	
	i = 0;
	
	for (e=self->target;e!=NULL;e=e->target)
	{
		e->targetX = self->x + (tiles[i] % boardWidth);
		e->targetY = self->y + (tiles[i] / boardWidth) * tileWidth;
		
		e->x = e->targetX;
		e->y = e->targetY;
		
		i++;
	}
}
