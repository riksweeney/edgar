/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../entity.h"
#include "../collisions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../player.h"
#include "../system/error.h"
#include "../audio/audio.h"

extern Entity *self, player;

static void createSnowball(void);
static void lookForPlayer(void);
static void throwSnowball(void);

Entity *addSasquatch(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Sasquatch");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	setEntityAnimation(self, WALK);

	moveLeftToRight();
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */
		
		if (collision(self->x + (self->face == LEFT ? -300 : self->w + 64), self->y, 232, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;
			
			self->thinkTime = 30;
			
			self->mental = 1 + prand() % 3;
			
			setEntityAnimation(self, ATTACK_1);
			
			self->action = &createSnowball;
		}
	}
}

static void createSnowball()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, ATTACK_2);
		
		self->action = throwSnowball;
		
		e = addProjectile("enemy/sasquatch_snowball", self, self->x, self->y, (self->face == RIGHT ? 8 : -8), 0);
		
		playSoundToMap("sound/common/throw.ogg", -1, self->x, self->y, 0);
		
		e->reactToBlock = &bounceOffShield;
		
		e->flags |= FLY;
		
		e->x += (self->face == RIGHT ? self->w : e->w);
		e->y += self->offsetY;
		
		self->thinkTime = 15;
	}
	
	checkToMap(self);
}

static void throwSnowball()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->mental--;
		
		if (self->mental > 0)
		{
			self->thinkTime = 30;
			
			setEntityAnimation(self, ATTACK_1);
			
			self->action = &createSnowball;
		}
		
		else
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;
			
			self->thinkTime = 120;
			
			self->action = &lookForPlayer;
		}
	}
}
