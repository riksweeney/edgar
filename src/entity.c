#include "headers.h"

#include "entity.h"
#include "collisions.h"
#include "item/item.h"
#include "custom_actions.h"
#include "decoration.h"
#include "event/trigger.h"
#include "event/global_trigger.h"
#include "system/properties.h"
#include "map.h"
#include "enemy/enemies.h"
#include "item/key_items.h"
#include "event/script.h"
#include "graphics/animation.h"
#include "player.h"

extern Entity *self, entity[MAX_ENTITIES];

static void scriptEntityMoveToTarget(void);
static void entityMoveToTarget(void);

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

			entity[i].weight = 1;

			entity[i].fallout = &removeEntity;

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

								self->dirY = sin(DEG_TO_RAD(self->startX)) / 20;
							}
						}

						if (self->dirY >= MAX_WATER_SPEED)
						{
							self->dirY = MAX_WATER_SPEED;
						}
					break;

					default:
						self->dirY += GRAVITY_SPEED * self->weight;

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
				if (self->standingOn != NULL)
				{
					self->dirX += self->standingOn->dirX;

					if (self->standingOn->dirY > 0)
					{
						self->dirY = self->standingOn->dirY + 1;
					}
				}

				self->action();

				self->standingOn = NULL;
			}

			else
			{
				checkToMap(self);
			}

			addToGrid(self);
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

	if (self->flags & PUSHABLE)
	{
		self->frameSpeed = 0;
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
	if (self->dirX == 0 || isAtEdge(self) == TRUE)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	checkToMap(self);
}

void flyLeftToRight()
{
	if (self->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	self->thinkTime += 5;

	self->dirY += sin(DEG_TO_RAD(self->thinkTime)) / 3;

	checkToMap(self);
}

void flyToTarget()
{
	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;
	}

	if (self->x == self->targetX)
	{
		self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
	}

	self->face = (self->dirX > 0 ? RIGHT : LEFT);

	self->thinkTime += 5;

	self->dirY += sin(DEG_TO_RAD(self->thinkTime)) / 3;

	checkToMap(self);
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

	setCustomAction(self, &invulnerable, 240);

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

		fireGlobalTrigger(self->objectiveName);
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
		setCustomAction(self, &invulnerable, 15);

		if (self->health > 0)
		{
			if (self->pain != NULL)
			{
				self->pain();
			}

			self->dirX = other->face == RIGHT ? 6 : -6;
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

	if (self->damage <= 0 || self->health <= 0)
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
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
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
			/*
			if (self->activate != NULL)
			{
				self->activate(1);
			}
			*/
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

				/*checkEntityToEntity(self);*/

				if (self->dirX == 0)
				{
					pushable = 0;
				}

				else
				{
					self->frameSpeed = 1;
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

				/*checkEntityToEntity(self);*/

				if (self->dirX == 0)
				{
					pushable = 0;
				}

				else
				{
					self->frameSpeed = -1;
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
			printf("Activating %s\n", entity[i].requires);

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

				printf("Activating %s (%s)\n", self->name, entity[i].name);

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
	int i, count;

	count = 0;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->inUse == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE %s\n", getEntityTypeByID(self->type));
			fprintf(fp, "NAME %s\n", self->name);
			fprintf(fp, "X %d\n", (int)self->x);
			fprintf(fp, "Y %d\n", (int)self->y);
			fprintf(fp, "START_X %d\n", (int)self->startX);
			fprintf(fp, "START_Y %d\n", (int)self->startY);
			fprintf(fp, "END_X %d\n", (int)self->endX);
			fprintf(fp, "END_Y %d\n", (int)self->endY);
			fprintf(fp, "MAX_THINKTIME %d\n", self->maxThinkTime);
			fprintf(fp, "THINKTIME %d\n", self->thinkTime);
			fprintf(fp, "HEALTH %d\n", self->health);
			if (self->type != WEAPON && self->type != SHIELD)
			{
				fprintf(fp, "DAMAGE %d\n", self->damage);
			}
			fprintf(fp, "SPEED %0.1f\n", self->speed);
			fprintf(fp, "WEIGHT %0.2f\n", self->weight);
			fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
			fprintf(fp, "REQUIRES %s\n", self->requires);
			fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
			fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
			fprintf(fp, "}\n\n");

			count++;
		}
	}

	printf("Total Entities in use: %d\n", count);
}

void addEntityFromScript(char *line)
{
	char entityType[MAX_VALUE_LENGTH], entityName[MAX_VALUE_LENGTH], objectiveName[MAX_VALUE_LENGTH];
	int x, y;
	Entity *e;

	sscanf(line, "%s %s \"%[^\"]\" %d %d", entityType, entityName, objectiveName, &x, &y);

	if (strcmpignorecase(entityType, "WEAPON") == 0 || strcmpignorecase(entityType, "SHIELD") == 0 ||
		strcmpignorecase(entityType, "ITEM") == 0)
	{
		e = addPermanentItem(entityName, x, y);

		if (strcmpignorecase(objectiveName, " ") != 0)
		{
			STRNCPY(e->objectiveName, objectiveName, sizeof(e->objectiveName));
		}
	}

	else if (strcmpignorecase(entityType, "KEY_ITEM") == 0)
	{
		addKeyItem(entityName, x, y);
	}

	else if (strcmpignorecase(entityType, "ENEMY") == 0)
	{
		addEnemy(entityName, x, y);
	}
}

void entityWalkTo(Entity *e, char *coords)
{
	int x, y;
	char wait[10];

	sscanf(coords, "%d %d %s", &x, &y, wait);

	e->targetX = x;
	e->targetY = y;

	if (!(e->flags & FLY))
	{
		e->targetY = e->y;
	}

	if (strcmpignorecase(wait, "WAIT") != 0)
	{
		e->action = &scriptEntityMoveToTarget;

		setScriptCounter(1);
	}

	else
	{
		e->action = &entityMoveToTarget;
	}

	setEntityAnimation(e, WALK);
}

void entityWalkToRelative(Entity *e, char *coords)
{
	int x, y;
	char wait[10];

	sscanf(coords, "%d %d %s", &x, &y, wait);

	e->targetX = self->x + x;
	e->targetY = self->y + y;

	if (!(e->flags & FLY))
	{
		e->targetY = e->y;
	}

	printf("%s will walk to %d %d\n", e->objectiveName, e->targetX, e->targetY);

	if (strcmpignorecase(wait, "WAIT") == 0)
	{
		e->action = &scriptEntityMoveToTarget;

		setScriptCounter(1);
	}

	else
	{
		e->action = &entityMoveToTarget;
	}

	setEntityAnimation(e, WALK);
}

static void scriptEntityMoveToTarget()
{
	self->face = (self->x < self->targetX) ? RIGHT : LEFT;

	printf("1. %d %d -> %d %d\n", (int)self->x, (int)self->y, self->targetX, self->targetY);

	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;
	}

	if (abs(self->y - self->targetY) > self->speed)
	{
		self->dirY = (self->y < self->targetY ? self->speed : -self->speed);
	}

	else
	{
		self->y = self->targetY;
	}

	if (self->x == self->targetX && self->y == self->targetY)
	{
		printf("%s reached Target\n", self->objectiveName);

		if (self->type == PLAYER)
		{
			self->action = &playerWaitForDialog;
		}

		setEntityAnimation(self, STAND);

		setScriptCounter(-1);
	}

	else
	{
		checkToMap(self);
	}
}

static void entityMoveToTarget()
{
	self->face = (self->x < self->targetX) ? RIGHT : LEFT;

	printf("2. %d %d -> %d %d\n", (int)self->x, (int)self->y, self->targetX, self->targetY);

	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;
	}

	if (abs(self->y - self->targetY) > self->speed)
	{
		self->dirY = (self->y < self->targetY ? self->speed : -self->speed);
	}

	else
	{
		self->y = self->targetY;
	}

	if (self->x == self->targetX && self->y == self->targetY)
	{
		setEntityAnimation(self, STAND);
	}

	checkToMap(self);
}
