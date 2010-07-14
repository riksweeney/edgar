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
#include "../entity.h"
#include "../custom_actions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../system/error.h"
#include "../audio/audio.h"
#include "splitter.h"

extern Entity *self;

static void bounceAround(void);
static void smallBounceAround(void);
static void takeDamage(Entity *, int);
static void mediumTakeDamage(Entity *, int);
static void smallTouch(Entity *);
static void smallDie(void);
static void mediumInit(void);
static void smallInit(void);
static void smallBecomeMedium(void);
static void mediumBecomeLarge(void);

Entity *addSplitter(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Splitter");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &bounceAround;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDieNoDrop;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;
	e->fallout = &entityDie;

	e->type = ENEMY;
	
	e->face = RIGHT;

	setEntityAnimation(e, STAND);

	return e;
}

Entity *addSplitterMedium(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Medium Splitter");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &bounceAround;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDieNoDrop;
	e->touch = &entityTouch;
	e->takeDamage = &mediumTakeDamage;
	e->fallout = &entityDie;

	e->type = ENEMY;
	
	e->face = RIGHT;

	setEntityAnimation(e, STAND);

	return e;
}

Entity *addSplitterSmall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Small Splitter");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &smallBounceAround;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &smallTouch;
	e->fallout = &entityDie;

	e->type = ENEMY;
	
	e->face = RIGHT;

	setEntityAnimation(e, STAND);

	return e;
}

static void bounceAround()
{
	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

		if (self->thinkTime == 0)
		{
			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2.ogg", -1, self->x, self->y, 0);
			}
			
			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1.ogg", -1, self->x, self->y, 0);
			}
			
			self->thinkTime = 30 + prand () % 60;
			
			self->dirY = -(6 + prand() % 2);
			
			if (prand() % 2 == 0)
			{
				self->face = LEFT;
				
				if (isAtEdge(self) == TRUE)
				{
					self->face = RIGHT;
				}
			}
			
			else
			{
				self->face = RIGHT;
				
				if (isAtEdge(self) == TRUE)
				{
					self->face = LEFT;
				}
			}
			
			self->dirX = (prand() % 2 + 2) * (self->face == LEFT ? -1 : 1);
		}

		else
		{
			self->thinkTime--;
		}
	}

	checkToMap(self);
}

static void mediumBounceAround()
{
	int y;
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
		
		if (self->mental <= 0)
		{
			y = self->y + self->h;
			
			loadProperties("enemy/splitter", self);
			
			setEntityAnimation(self, CUSTOM_1);
			
			self->y = y - self->h;
			
			self->action = &mediumBecomeLarge;
			
			self->thinkTime = 120;
			
			setCustomAction(self, &invulnerableNoFlash, 120, 0, 0);
		}
		
		else if (self->thinkTime == 0)
		{
			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2.ogg", -1, self->x, self->y, 0);
			}
			
			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1.ogg", -1, self->x, self->y, 0);
			}
			
			self->thinkTime = 30 + prand () % 60;
			
			self->dirY = -(6 + prand() % 2);
			
			if (prand() % 2 == 0)
			{
				self->face = LEFT;
				
				if (isAtEdge(self) == TRUE)
				{
					self->face = RIGHT;
				}
			}
			
			else
			{
				self->face = RIGHT;
				
				if (isAtEdge(self) == TRUE)
				{
					self->face = LEFT;
				}
			}
			
			self->dirX = (prand() % 2 + 2) * (self->face == LEFT ? -1 : 1);
		}

		else
		{
			self->thinkTime--;
			
			self->mental--;
		}
	}

	checkToMap(self);
}

static void smallBounceAround()
{
	int y;

	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
		
		if (self->mental <= 0)
		{
			y = self->y + self->h;
			
			loadProperties("enemy/splitter_medium", self);
			
			setEntityAnimation(self, CUSTOM_1);
			
			self->y = y - self->h;
			
			self->action = &smallBecomeMedium;
			
			self->thinkTime = 120;
			
			setCustomAction(self, &invulnerableNoFlash, 120, 0, 0);
		}
		
		else if (self->thinkTime == 0)
		{
			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/baby_jump2.ogg", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/baby_jump1.ogg", -1, self->x, self->y, 0);
			}
			
			self->dirY = -(6 + prand() % 2);
			
			self->thinkTime = 30 + prand () % 60;
			
			if (prand() % 2 == 0)
			{
				self->face = LEFT;
				
				if (isAtEdge(self) == TRUE)
				{
					self->face = RIGHT;
				}
			}
			
			else
			{
				self->face = RIGHT;
				
				if (isAtEdge(self) == TRUE)
				{
					self->face = LEFT;
				}
			}
			
			self->dirX = (prand() % 2 + 2) * (self->face == LEFT ? -1 : 1);
		}

		else
		{
			self->thinkTime--;
			
			self->mental--;
		}
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *e;
	
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
			}
		}

		else
		{
			e = addSplitterMedium(self->x, self->y, "enemy/splitter_medium");
			
			e->action = &mediumInit;
			
			setCustomAction(e, &invulnerable, 30, 0, 0);
			
			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;
			
			e->dirX = (20 + prand() % 20) * (other->x < self->x ? 1 : -1);
			e->dirY = -(30 + prand() % 60);
			
			e->dirX /= 10;
			e->dirY /= 10;
			
			e = addSplitterMedium(self->x, self->y, "enemy/splitter_medium");
			
			e->action = &mediumInit;
			
			setCustomAction(e, &invulnerable, 30, 0, 0);
			
			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;
			
			e->dirX = (20 + prand() % 20) * (other->x < self->x ? 1 : -1);
			e->dirY = -(30 + prand() % 60);
			
			e->dirX /= 10;
			e->dirY /= 10;
			
			self->inUse = FALSE;
		}
		
		enemyPain();
	}
}

static void mediumTakeDamage(Entity *other, int damage)
{
	Entity *e;
	
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
			}
		}

		else
		{
			e = addSplitterSmall(self->x, self->y, "enemy/splitter_small");
			
			e->action = &smallInit;
			
			setCustomAction(e, &invulnerable, 30, 0, 0);
			
			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;
			
			e->dirX = (20 + prand() % 20) * (other->x < self->x ? 1 : -1);
			e->dirY = -(30 + prand() % 60);
			
			e->dirX /= 10;
			e->dirY /= 10;
			
			self->inUse = FALSE;
			
			e = addSplitterSmall(self->x, self->y, "enemy/splitter_small");
			
			e->action = &smallInit;
			
			setCustomAction(e, &invulnerable, 30, 0, 0);
			
			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;
			
			e->dirX = (20 + prand() % 20) * (other->x < self->x ? 1 : -1);
			e->dirY = -(30 + prand() % 60);
			
			e->dirX /= 10;
			e->dirY /= 10;
			
			self->inUse = FALSE;
		}
		
		enemyPain();
	}
}

static void smallTouch(Entity *other)
{
	if (other->type == PLAYER && other->dirY > 0)
	{
		self->touch = &entityTouch;
		
		playSoundToMap("sound/enemy/splitter/splat.ogg", -1, self->x, self->y, 0);
		
		setEntityAnimation(self, DIE);
		
		self->thinkTime = 120;
		
		self->action = &smallDie;
		
		self->dirX = 0;
	}
}

static void smallDie()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		dropRandomItem(self->x + self->w / 2, self->y);
		
		self->inUse = FALSE;
	}
	
	checkToMap(self);
}

static void mediumInit()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
		
		self->action = &mediumBounceAround;
	}
}

static void smallInit()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
		
		self->action = &smallBounceAround;
	}
}

static void mediumBecomeLarge()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, STAND);
		
		self->action = &bounceAround;
		self->touch = &entityTouch;
		self->takeDamage = &takeDamage;
	}
	
	checkToMap(self);
}

static void smallBecomeMedium()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, STAND);
		
		self->action = &mediumBounceAround;
		self->touch = &entityTouch;
		self->takeDamage = &mediumTakeDamage;
	}
	
	checkToMap(self);
}