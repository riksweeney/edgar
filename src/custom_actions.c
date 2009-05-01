#include "headers.h"

extern Entity *self;

void setCustomAction(Entity *e, void (*func)(int *, int *), int thinkTime, int accumulates)
{
	int i;
	Entity *temp;

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		/* Search for an already existing action */

		if (e->customAction[i].thinkTime != 0 && e->customAction[i].action == func)
		{
			if (thinkTime >= e->customAction[i].thinkTime)
			{
				e->customAction[i].thinkTime = thinkTime;

				e->customAction[i].counter += accumulates;
			}

			return;
		}
	}

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		if (e->customAction[i].thinkTime == 0)
		{
			e->customAction[i].action = func;

			e->customAction[i].thinkTime = thinkTime + 1;

			e->customAction[i].counter = accumulates;

			/* Execute the custom action once */

			temp = self;

			self = e;

			self->customAction[i].action(&self->customAction[i].thinkTime, &self->customAction[i].counter);

			self = temp;

			return;
		}
	}

	printf("No free slots for Custom Action\n");

	exit(1);
}

void doCustomAction(CustomAction *customAction)
{
	customAction->action(&customAction->thinkTime, &customAction->counter);
}

void helpless(int *thinkTime, int *counter)
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

void invulnerable(int *thinkTime, int *counter)
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

void invulnerableNoFlash(int *thinkTime, int *counter)
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
}

void flashWhite(int *thinkTime, int *counter)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}
}

void slowDown(int *thinkTime, int *counter)
{
	(*thinkTime)--;

	if (*counter <= 0)
	{
		(*counter) = 1;
	}

	if (*thinkTime != 0)
	{
		self->frameSpeed = 1.0f / (*counter);
		self->speed = self->originalSpeed / (*counter);
	}

	else
	{
		self->frameSpeed = 1;
		self->speed = self->originalSpeed;
	}
}
