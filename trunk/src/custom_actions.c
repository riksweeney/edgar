#include "custom_actions.h"

void setCustomAction(Entity *e, void (*func)(int *), int thinkTime)
{
	int i;
	Entity *temp;

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		if (e->customThinkTime[i] == 0)
		{
			e->custom[i] = func;
			
			e->customThinkTime[i] = thinkTime + 1;
			
			/* Execute the custom action once */
			
			temp = self;
			
			self = e;
			
			self->custom[i](&e->customThinkTime[i]);
			
			self = temp;
			
			return;
		}
	}
	
	printf("No free slots for Custom Action\n");
	
	exit(1);
}

void helpless(int *thinkTime)
{
	(*thinkTime)--;
	
	if (*thinkTime != 0)
	{
		self->flags |= HELPLESS;
	}
	
	else
	{
		self->flags &= ~HELPLESS;
		
		self->dirX = 0;
		
		if (self->flags & FLY)
		{
			self->dirY = 0;
		}
	}
}

void invulnerable(int *thinkTime)
{
	(*thinkTime)--;
	
	if (*thinkTime != 0)
	{
		self->flags |= INVULNERABLE;
	}
	
	else
	{
		self->flags &= ~INVULNERABLE;
	}
	
	if ((*thinkTime) % 3 == 0)
	{
		self->flags ^= NO_DRAW;
	}
	
	if ((*thinkTime) == 0)
	{
		self->flags &= ~NO_DRAW;
	}
}
