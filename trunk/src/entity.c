#include "entity.h"

extern int collision(int, int, int, int, int, int, int, int);
extern void checkToMap(Entity *);
extern void dropRandomItem(int, int);
extern void helpless(int *);
extern void invulnerable(int *);
extern void setCustomAction(Entity *, void (*)(int *), int);

void standardDie(void);

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
	int i, j;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->active == ACTIVE && !(self->flags & STATIC))
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
	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;
	}

	self->dirX = 0;

	checkToMap(self);
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
		self->active = INACTIVE;
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

void entityFallOnOther(Entity *other)
{
	if ((other->flags & TRAPPABLE) && self->dirY > 0)
	{
		self->y -= self->dirY;

		if (collision(self->x, self->y, self->w, self->h, other->x, other->y, other->w, other->h) == 0)
		{
			other->flags |= HELPLESS|NO_DRAW;
		}

		self->y += self->dirY;
	}
}

int addEntity(Entity e, int x, int y)
{
	int i;

	/* Loop through the entities and perform their action */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].active == INACTIVE)
		{
			entity[i] = e;

			entity[i].currentFrame = 0;

			entity[i].active = ACTIVE;

			entity[i].x = x;

			entity[i].y = y;

			return 1;
		}
	}

	return 0;
}

