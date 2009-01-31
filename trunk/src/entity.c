#include "headers.h"

#include "entity.h"
#include "collisions.h"
#include "item.h"
#include "custom_actions.h"

extern Entity *self, entity[MAX_ENTITIES];

void clearEntities()
{
	/* Clear the list */

	memset(entity, 0, sizeof(Entity) * MAX_ENTITIES);
}

Entity *getFreeEntity()
{
	int i;

	/* Loop through all the entities and find a free slot */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == NOT_IN_USE)
		{
			memset(&entity[i], 0, sizeof(Entity));

			entity[i].inUse = IN_USE;

			entity[i].active = ACTIVE;

			return &entity[i];
		}
	}

	/* Return NULL if you couldn't any free slots */

	return NULL;
}

void doEntities()
{
	int i, j;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->inUse == IN_USE && !(self->flags & STATIC))
		{
			for (j=0;j<MAX_CUSTOM_ACTIONS;j++)
			{
				if (self->customThinkTime[j] > 0)
				{
					self->custom[j](&self->customThinkTime[j]);
				}
			}

			if (!(self->flags & FLY))
			{
				self->dirY += GRAVITY_SPEED;

				if (self->dirY >= MAX_FALL_SPEED)
				{
					self->dirY = MAX_FALL_SPEED;
				}
			}
			
			else
			{
				self->dirY = 0;
			}

			if (!(self->flags & HELPLESS))
			{
				if (self->standingOn != NULL)
				{
					self->dirX += self->standingOn->dirX;
	
					if (self->standingOn->dirY > 0)
					{
						self->dirY = self->standingOn->dirY;
					}
					
					else
					{
						self->dirY = 0;
					}
				}
				
				self->action();
			}
		}
	}
}

void drawEntities(int drawAll)
{
	int i;

	if (drawAll == 0)
	{
		/* Draw standard entities */

		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == IN_USE && !(self->flags & NO_DRAW) && !(self->flags & ALWAYS_ON_TOP))
			{
				self->draw();
			}
		}

		/* Draw entities that must appear at the front */

		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == IN_USE && !(self->flags & NO_DRAW) && (self->flags & ALWAYS_ON_TOP))
			{
				self->draw();
			}
		}
	}

	else
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == IN_USE)
			{
				self->draw();
			}
		}
	}
}

void removeEntity()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = NOT_IN_USE;
	}
}

void doNothing(void)
{
	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;
	}

	self->dirX = 0;

	checkToMap(self);
	
	self->standingOn = NULL;
}

void entityDie()
{
	dropRandomItem(self->x + self->w / 2, self->y);

	self->flags &= ~FLY;

	self->thinkTime = 120;

	setCustomAction(self, &invulnerable, 120);

	self->action = &standardDie;
}

void standardDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = NOT_IN_USE;
	}

	self->dirX = 0;

	self->dirY += GRAVITY_SPEED;

	if (self->dirY >= MAX_FALL_SPEED)
	{
		self->dirY = MAX_FALL_SPEED;
	}

	checkToMap(self);
}

void entityTakeDamage(Entity *other, int damage)
{
	if (damage != 0)
	{
		self->health -= damage;

		setCustomAction(self, &helpless, 10);
		setCustomAction(self, &invulnerable, 10);

		if (self->health > 0)
		{
			if (self->dirX == 0)
			{
				self->dirX = other->dirX < 0 ? -12 : 12;
			}

			else if (other->dirX == 0)
			{
				self->dirX = other->face == RIGHT ? 12 : -12;
			}

			else
			{
				self->dirX = self->dirX < 0 ? 12 : -12;
			}
		}

		else
		{
			self->die();
		}
	}
}

void entityTouch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}

	else if (other->type == WEAPON)
	{
		self->takeDamage(other, other->damage);
	}
}

void pushEntity(Entity *other)
{
	int pushable;

	other->x -= other->dirX;
	other->y -= other->dirY;

	pushable = (self->flags & PUSHABLE);

	/* Test the horizontal movement */

	if (other->dirX > 0)
	{
		/* Trying to move right */

		if (collision(other->x + other->dirX, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			if (pushable != 0)
			{
				self->dirX += other->dirX;

				checkToMap(self);

				if (self->dirX == 0)
				{
					pushable = 0;
				}
			}

			if (pushable == 0)
			{
				/* Place the entity as close as possible */

				other->x = self->x;
				other->x -= other->w;

				other->dirX = 0;
			}
		}
	}

	else if (other->dirX < 0)
	{
		/* Trying to move left */

		if (collision(other->x + other->dirX, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			if (pushable != 0)
			{
				self->dirX += other->dirX;

				checkToMap(self);

				if (self->dirX == 0)
				{
					pushable = 0;
				}
			}

			if (pushable == 0)
			{
				/* Place the entity as close as possible */

				other->x = self->x;
				other->x += self->w;

				other->dirX = 0;
			}
		}
	}

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y + other->dirY, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			/* Place the entity as close as possible */

			other->y = self->y;
			other->y -= other->h;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;

			if (self->activate != NULL)
			{
				self->activate(1);
			}
		}
	}

	else if (other->dirY < 0)
	{
		/* Trying to move up */

		if (collision(other->x, other->y + other->dirY, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			/* Place the entity as close as possible */

			other->y = self->y;
			other->y += self->h;

			other->dirY = 0;
		}
	}

	other->x += other->dirX;
	other->y += other->dirY;
}

int addEntity(Entity e, int x, int y)
{
	int i;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == NOT_IN_USE)
		{
			entity[i] = e;

			entity[i].currentFrame = 0;

			entity[i].inUse = IN_USE;

			entity[i].x = x;

			entity[i].y = y;

			return 1;
		}
	}

	return 0;
}

Entity *getEntityByObjectiveName(char *name)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == IN_USE && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			return &entity[i];
		}
	}

	return NULL;
}

void activateEntitiesWithName(char *name, int val)
{
	int i;
	Entity *e;
	
	if (name == NULL || strlen(name) == 0)
	{
		printf("Name is blank!\n");
		
		exit(1);
	}

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == IN_USE && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			e = self;
			
			self = &entity[i];
			
			self->activate(val);
			
			self = e;
		}
	}
}
