#include "item.h"

extern void checkToMap(Entity *);

void dropItem(int item)
{
	
}

void generalItemAction()
{
	self->dirY += GRAVITY_SPEED;

	if (self->dirY >= MAX_FALL_SPEED)
	{
		self->dirY = MAX_FALL_SPEED;
	}
	
	self->thinkTime--;
	
	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}
	
	if (self->thinkTime == 0)
	{
		self->active = INACTIVE;
	}
	
	checkToMap(self);
	
	self->standingOn = NULL;
}

void healthTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->health += self->health;
		
		if (other->health > other->maxHealth)
		{
			other->health = other->maxHealth;
		}
		
		self->active = INACTIVE;
	}
}
