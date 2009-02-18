#include "headers.h"

#include "entity.h"
#include "collisions.h"
#include "item.h"
#include "custom_actions.h"
#include "decoration.h"
#include "trigger.h"
#include "properties.h"

extern Entity *self, entity[MAX_ENTITIES];

void freeEntities()
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
		if (entity[i].inUse == FALSE)
		{
			memset(&entity[i], 0, sizeof(Entity));

			entity[i].inUse = TRUE;

			entity[i].active = TRUE;

			entity[i].frameSpeed = 1;

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

		if (self->inUse == TRUE)
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
				switch (self->environment)
				{	
					case WATER:
						self->dirY += GRAVITY_SPEED * 0.25;
						
						if (self->flags & FLOATS)
						{
							if (self->dirX != 0)
							{
								self->startX++;
								
								self->dirY = sin(self->startX * PI / 180) / 10;
							}
						}
						
						if (self->dirY >= MAX_WATER_SPEED)
						{
							self->dirY = MAX_WATER_SPEED;
						}
					break;
					
					default:
						self->dirY += GRAVITY_SPEED;
						
						if (self->dirY >= MAX_AIR_SPEED)
						{
							self->dirY = MAX_AIR_SPEED;
						}
		
						else if (self->dirY > 0 && self->dirY < 1)
						{
							self->dirY = 1;
						}
					break;
				}
			}

			else
			{
				self->dirY = 0;
			}

			if (!(self->flags & HELPLESS))
			{
				self->action();

				self->standingOn = NULL;
			}

			else
			{
				checkToMap(self);
			}
		}
	}
}

void drawDoors()
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->inUse == TRUE && !(self->flags & NO_DRAW) && !(self->flags & ALWAYS_ON_TOP))
		{
			self->draw();
		}
	}
}

void drawEntities(int drawAll)
{
	int i;

	if (drawAll == FALSE)
	{
		/* Draw standard entities */

		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == TRUE && !(self->flags & NO_DRAW) && !(self->flags & ALWAYS_ON_TOP))
			{
				self->draw();
			}
		}

		/* Draw entities that must appear at the front */

		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == TRUE && !(self->flags & NO_DRAW) && (self->flags & ALWAYS_ON_TOP))
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

			if (self->inUse == TRUE)
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
		self->inUse = FALSE;
	}
}

void doNothing(void)
{
	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;
	}

	if (!(self->flags & HELPLESS))
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	}
	
	checkToMap(self);
	
	if (self->environment == WATER && (self->flags & FLOATS))
	{
		self->action = &floatLeftToRight;
		
		self->endX = self->dirX = 0.5;
		
		self->thinkTime = 0;
	}

	self->standingOn = NULL;
}

void moveLeftToRight()
{
	self->dirX = self->face == RIGHT ? self->speed : -self->speed;

	if (isAtEdge(self) == TRUE)
	{
		self->face = self->face == RIGHT ? LEFT : RIGHT;

		self->dirX = self->face == RIGHT ? self->speed : -self->speed;
	}

	checkToMap(self);

	self->standingOn = NULL;
}

void floatLeftToRight()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime == 0)
		{
			self->dirX = self->endX;
		}
	}

	else
	{
		checkToMap(self);

		if (self->dirX == 0)
		{
			self->endX *= -1;

			self->thinkTime = 120;
		}
	}
}

void entityDie()
{
	self->flags &= ~FLY;

	self->thinkTime = 60;

	setCustomAction(self, &invulnerable, 60);

	self->action = &standardDie;
}

void standardDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;

		dropRandomItem(self->x + self->w / 2, self->y);

		fireTrigger(self->objectiveName);
	}

	self->dirX = 0;

	checkToMap(self);
}

void entityTakeDamage(Entity *other, int damage)
{
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		setCustomAction(self, &helpless, 10);
		setCustomAction(self, &invulnerable, 10);

		if (self->health > 0)
		{
			if (self->pain != NULL)
			{
				self->pain();
			}

			self->dirX = other->face == RIGHT ? 12 : -12;
		}

		else
		{
			self->damage = 0;

			self->die();
		}
	}
}

void entityTouch(Entity *other)
{
	Entity *temp;

	if (self->damage <= 0)
	{
		return;
	}

	if (other->type == PLAYER && self->parent != other)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL)
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && self->parent != other)
	{
		if (self->takeDamage != NULL)
		{
			self->takeDamage(other, other->damage);
		}

		other->inUse = FALSE;
	}
}

void pushEntity(Entity *other)
{
	int pushable;

	if (other->type == MANUAL_DOOR || other->type == AUTO_DOOR || other->type == AUTO_LIFT || other->type == MANUAL_LIFT)
	{
		return;
	}

	other->x -= other->dirX;
	other->y -= other->dirY;

	pushable = (self->flags & PUSHABLE);

	if (self->flags & OBSTACLE)
	{
		pushable = 0;
	}

	/* Test the vertical movement */

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

				checkEntityToEntity(self);

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

				if ((other->flags & GRABBING) && other->target != NULL)
				{
					other->target->x -= other->target->dirX;
					other->target->dirX = 0;
				}
			}

			if ((other->flags & GRABBING) && other->target == NULL && (self->flags & PUSHABLE))
			{
				other->target = self;

				self->flags |= HELPLESS;
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

				checkEntityToEntity(self);

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

				if ((other->flags & GRABBING) && other->target != NULL)
				{
					other->target->x -= other->target->dirX;
					other->target->dirX = 0;
				}
			}

			if ((other->flags & GRABBING) && other->target == NULL && (self->flags & PUSHABLE))
			{
				other->target = self;

				self->flags |= HELPLESS;
			}
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
		if (entity[i].inUse == FALSE)
		{
			entity[i] = e;

			entity[i].currentFrame = 0;

			entity[i].inUse = TRUE;

			entity[i].x = x;

			entity[i].y = y;

			return TRUE;
		}
	}

	return FALSE;
}

Entity *getEntityByObjectiveName(char *name)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			return &entity[i];
		}
	}

	return NULL;
}

void activateEntitiesWithName(char *name, int active)
{
	int i;

	if (name == NULL || strlen(name) == 0)
	{
		printf("Name is blank!\n");

		exit(1);
	}

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].requires, name) == 0)
		{
			entity[i].active = active;
		}
	}
}

void interactWithEntity(int x, int y, int w, int h)
{
	int i;
	Entity *e;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].activate != NULL)
		{
			if (collision(x, y, w, h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
			{
				e = self;

				self = &entity[i];

				self->activate(1);

				self = e;
			}
		}
	}
}

void initLineDefs()
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == LINE_DEF)
		{
			self = &entity[i];

			self->flags &= ~NO_DRAW;

			self->action();
		}
	}
}

void writeEntitiesToFile(FILE *fp)
{
	int i;
	char type[MAX_VALUE_LENGTH];

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->inUse == TRUE)
		{
			strcpy(type, getTypeByID(self->type));

			fprintf(fp, "{\n");
			fprintf(fp, "TYPE %s\n", type);
			fprintf(fp, "NAME %s\n", self->name);
			fprintf(fp, "START_X %d\n", (int)self->x);
			fprintf(fp, "START_Y %d\n", (int)self->y);
			fprintf(fp, "END_X %d\n", (int)self->endX);
			fprintf(fp, "END_Y %d\n", (int)self->endY);
			fprintf(fp, "THINKTIME %d\n", self->thinkTime);
			fprintf(fp, "HEALTH %d\n", self->health);
			fprintf(fp, "SPEED %0.1f\n", self->speed);
			fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
			fprintf(fp, "REQUIRES %s\n", self->requires);
			fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
			fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
			fprintf(fp, "}\n\n");
		}
	}
}
