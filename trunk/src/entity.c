#include "entity.h"

extern int collision(int, int, int, int, int, int, int, int);
extern void checkToMap(Entity *);

void clearEntities()
{
	int i;

	/* Clear the list */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		memset(entity, 0, sizeof(Entity) * MAX_ENTITIES);
	}
}

Entity *getFreeEntity()
{
	int i;

	/* Loop through all the entities and find a free slot */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].active == INACTIVE)
		{
			memset(&entity[i], 0, sizeof(Entity));

			entity[i].active = ACTIVE;

			return &entity[i];
		}
	}

	/* Return NULL if you couldn't any free slots */

	return NULL;
}

void doEntities()
{
	int i;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->active == ACTIVE && !(self->flags & STATIC))
		{
			self->action();
		}
	}
}

void drawEntities()
{
	int i;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->active == ACTIVE && !(self->flags & NO_DRAW))
		{
			self->draw();
		}
	}
}

Entity *getEntityAtLocation(int x, int y)
{
	int i;

	/* Loop through the entities */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if ((int)entity[i].x / TILE_SIZE == x && (int)entity[i].y / TILE_SIZE == y)
		{
			return &entity[i];
		}
	}
	
	return NULL;
}

void removeEntity()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->active = INACTIVE;
	}
}

void doNothing()
{
	self->dirX = 0;
	
	self->dirY += GRAVITY_SPEED;

	if (self->dirY >= MAX_FALL_SPEED)
	{
		self->dirY = MAX_FALL_SPEED;
	}
	
	checkToMap(self);
}

void pushEntity(Entity *other)
{
	int pushable;
	
	other->x -= other->dirX;
	other->y -= other->dirY;
	
	pushable = 1;
	
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
				/* Place the player as close to the solid tile as possible */
				
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
				/* Place the player as close to the solid tile as possible */
				
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
			/* Place the player as close to the solid tile as possible */
			
			other->y = self->y;
			other->y -= other->h;
			
			other->dirY = 0;
			other->flags |= ON_GROUND;
		}
	}
	
	other->x += other->dirX;
	other->y += other->dirY;
}

void addEntity(Entity e)
{
	int i;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].active == INACTIVE)
		{
			entity[i] = e;
			
			return;
		}
	}
	
	printf("Could not add an Entity\n");
	
	exit(1);
}

