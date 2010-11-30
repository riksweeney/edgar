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

#include "headers.h"

#include "entity.h"
#include "collisions.h"
#include "item/item.h"
#include "custom_actions.h"
#include "graphics/decoration.h"
#include "event/trigger.h"
#include "event/global_trigger.h"
#include "system/properties.h"
#include "system/resources.h"
#include "map.h"
#include "enemy/enemies.h"
#include "item/key_items.h"
#include "event/script.h"
#include "graphics/animation.h"
#include "player.h"
#include "system/random.h"
#include "audio/audio.h"
#include "system/pak.h"
#include "geometry.h"
#include "hud.h"
#include "system/error.h"
#include "game.h"
#include "world/action_point.h"
#include "world/falling_platform.h"
#include "world/weak_wall.h"

extern Entity *self, entity[MAX_ENTITIES];

static int entityIndex = 0, drawLayerIndex[MAX_LAYERS];
static Entity *drawLayer[MAX_LAYERS][MAX_ENTITIES];

static void scriptEntityMoveToTarget(void);
static void entityMoveToTarget(void);
static void scriptDoNothing(void);

void freeEntities()
{
	/* Clear the list */

	memset(entity, 0, sizeof(Entity) * MAX_ENTITIES);

	entityIndex = 0;
}

Entity *getFreeEntity()
{
	int i, count;

	count = 0;

	/* Loop through all the entities and find a free slot */

	for (i=entityIndex;;i++)
	{
		if (i >= MAX_ENTITIES)
		{
			i = 0;
		}

		if (entity[i].inUse == FALSE)
		{
			memset(&entity[i], 0, sizeof(Entity));

			entity[i].inUse = TRUE;

			entity[i].active = TRUE;

			entity[i].frameSpeed = 1;

			entity[i].weight = 1;

			entity[i].originalWeight = 1;

			entity[i].fallout = NULL;

			entity[i].currentAnim = -1;

			entity[i].layer = MID_GROUND_LAYER;

			entity[i].alpha = 255;

			entityIndex = i + 1;

			return &entity[i];
		}

		count++;

		if (count >= MAX_ENTITIES - 20)
		{
			printf("WARNING, compacting Entities!\n");

			if (count == MAX_ENTITIES)
			{
				break;
			}
		}
	}

	/* Return NULL if you couldn't any free slots */

	return NULL;
}

void resetEntityIndex()
{
	entityIndex = 0;
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
			self->flags &= ~(HELPLESS|INVULNERABLE|FLASH|ATTRACTED);

			for (j=0;j<MAX_CUSTOM_ACTIONS;j++)
			{
				if (self->customAction[j].thinkTime > 0)
				{
					doCustomAction(&self->customAction[j]);
				}
			}

			if (!(self->flags & TELEPORTING))
			{
				if (!(self->flags & (FLY|GRABBED)))
				{
					switch (self->environment)
					{
						case WATER:
						case SLIME:
							self->dirY += GRAVITY_SPEED * 0.25 * self->weight;

							if (self->flags & FLOATS)
							{
								if (self->dirX != 0)
								{
									self->endY++;

									self->dirY = cos(DEG_TO_RAD(self->endY)) / 20;
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

				if (self->flags & GRABBED)
				{
					self->dirY = 0;
				}

				if (self->standingOn != NULL)
				{
					if (self->standingOn->dirX != 0)
					{
						self->dirX = self->standingOn->dirX;
					}

					if (self->standingOn->dirY > 0)
					{
						self->dirY = self->standingOn->dirY + 1;
					}
				}

				if (!(self->flags & HELPLESS))
				{
					if (self->action == NULL)
					{
						showErrorAndExit("%s has no action function", self->name);
					}

					self->action();
				}

				else
				{
					checkToMap(self);
				}

				self->standingOn = NULL;

				if (self->flags & SPAWNED_IN)
				{
					self->spawnTime--;
				}

				if (self->inUse == TRUE)
				{
					addToGrid(self);
				}
			}

			else
			{
				doTeleport();
			}

			addToDrawLayer(self, self->layer);
		}
	}
}

void drawEntities(int depth)
{
	int i, drawn;

	/* Draw standard entities */

	if (depth == -1)
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = &entity[i];

			if (self->inUse == TRUE)
			{
				if (self->draw == NULL)
				{
					showErrorAndExit("%s has no draw function", self->name);
				}

				self->draw();
			}
		}
	}

	else
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			self = drawLayer[depth][i];

			if (self == NULL)
			{
				break;
			}

			if (self->inUse == TRUE && !(self->flags & NO_DRAW) && self->layer == depth)
			{
				if (self->draw == NULL)
				{
					showErrorAndExit("%s has no draw function", self->name);
				}

				drawn = self->draw();

				/* Live for 2 minutes whilst on the screen */

				if (drawn == TRUE && (self->flags & SPAWNED_IN))
				{
					self->spawnTime = SPAWNED_IN_TIME;
				}

				else if (drawn == FALSE && (self->flags & SPAWNED_IN) && self->spawnTime <= 0 && (self->spawnTime % 60 == 0))
				{
					/* Teleport expired enemies beneath the map */

					if (self->health != 0)
					{
						self->y = (MAX_MAP_Y + 1) * TILE_SIZE;

						self->action = &entityDieNoDrop;
					}
				}
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

void doNothing()
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

	if (!(self->flags & GRABBED))
	{
		if (self->flags & PUSHABLE)
		{
			self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;
		}

		else
		{
			self->dirX = self->standingOn != NULL ? self->standingOn->dirX : (self->flags & ON_GROUND ? 0 : self->dirX);
		}

		if (self->standingOn != NULL)
		{
			self->dirX = self->standingOn->dirX;

			if (self->standingOn->dirY > 0)
			{
				self->dirY = self->standingOn->dirY + 1;
			}

			else if (self->standingOn->dirY == 0 && self->standingOn->dirX == 0)
			{
				self->dirY = 0;
			}

			self->flags |= ON_GROUND;
		}
	}

	else if (self->dirX != 0)
	{
		self->frameSpeed = self->dirX < 0 ? -1 : 1;
	}

	checkToMap(self);

	if (self->environment == WATER && (self->flags & FLOATS))
	{
		self->action = &floatLeftToRight;

		self->endX = self->dirX = 1.0;

		self->thinkTime = 0;
	}

	self->standingOn = NULL;
}

void moveLeftToRight()
{
	if (self->dirX == 0)
	{
		self->x += self->face == LEFT ? self->box.x : -self->box.x;

		self->face = self->face == RIGHT ? LEFT : RIGHT;
	}

	if (self->standingOn == NULL || self->standingOn->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	else
	{
		self->dirX += (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (isAtEdge(self) == TRUE)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}
}

void flyLeftToRight()
{
	if (self->dirX == 0)
	{
		self->x += self->face == LEFT ? self->box.x : -self->box.x;

		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	self->thinkTime += 5;

	self->dirY += cos(DEG_TO_RAD(self->thinkTime)) / 3;

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
		self->dirX = self->endX;

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
	self->damage = 0;

	self->health = 0;

	if (!(self->flags & INVULNERABLE))
	{
		self->flags &= ~FLY;

		self->weight = fabs(self->originalWeight);

		self->flags |= DO_NOT_PERSIST;

		self->thinkTime = 60;

		setCustomAction(self, &invulnerable, 240, 0, 0);

		self->frameSpeed = 0;

		self->action = &standardDie;

		self->damage = 0;
	}
}

void standardDie()
{
	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		self->inUse = FALSE;

		dropRandomItem(self->x + self->w / 2, self->y);
	}

	checkToMap(self);
}

void entityDieNoDrop()
{
	self->damage = 0;

	self->health = 0;

	if (!(self->flags & INVULNERABLE))
	{
		self->flags &= ~FLY;

		self->weight = fabs(self->originalWeight);

		self->flags |= DO_NOT_PERSIST;

		self->thinkTime = 60;

		setCustomAction(self, &invulnerable, 240, 0, 0);

		self->frameSpeed = 0;

		self->action = &noItemDie;

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);
	}
}

void entityDieVanish()
{
	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	self->inUse = FALSE;
}

void noItemDie()
{
	if (self->flags & ON_GROUND)
	{
		self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		self->inUse = FALSE;
	}

	checkToMap(self);
}

void entityTakeDamageFlinch(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &helpless, 10, 0, 0);
			setCustomAction(self, &invulnerable, HIT_INVULNERABLE_TIME, 0, 0);

			if (self->pain != NULL)
			{
				self->pain();
			}

			self->dirX = other->face == RIGHT ? 6 : -6;
		}

		else
		{
			self->damage = 0;

			if (other->type == WEAPON || other->type == PROJECTILE)
			{
				increaseKillCount();
			}

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		addDamageScore(damage, self);
	}
}

void entityTakeDamageNoFlinch(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			self->damage = 0;

			if (other->type == WEAPON || other->type == PROJECTILE)
			{
				increaseKillCount();
			}

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		addDamageScore(damage, self);
	}
}

void enemyPain()
{
	int i = prand() % 3;

	switch (i)
	{
		case 0:
			playSoundToMap("sound/common/splat1.ogg", -1, self->x, self->y, 0);
		break;

		case 1:
			playSoundToMap("sound/common/splat2.ogg", -1, self->x, self->y, 0);
		break;

		default:
			playSoundToMap("sound/common/splat3.ogg", -1, self->x, self->y, 0);
		break;
	}
}

void entityTouch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER && self->parent != other && self->damage != 0)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}

	else if ((other->type == EXPLOSION || (other->type == WEAPON && (other->flags & ATTACKING))) && other->damage != 0)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self && self->health > 0)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}
}

void pushEntity(Entity *other)
{
	int pushable, collided;
	int x1, x2, y1, y2;
	Entity *temp;
	float dirX;
	static int depth = 0;
	long wasOnGround;

	if (other->touch == NULL || (other->type == WEAPON && other->flags & ATTACKING) || (other->flags & PLAYER_TOUCH_ONLY))
	{
		return;
	}

	if (other->flags & OBSTACLE)
	{
		return;
	}

	if (other->type == PROJECTILE)
	{
		temp = self;

		self = other;

		self->die();

		self = temp;

		return;
	}

	other->x -= other->dirX;
	other->y -= other->dirY;
	/*
	if (other->type == PLAYER && other->dirX > 0)
	{
		other->x = ceil(other->x);
	}

	else if (other->type == PLAYER && other->dirX < 0)
	{
		other->x = floor(other->x);
	}
	*/
	x1 = getLeftEdge(self);

	y1 = self->y + self->box.y;

	x2 = getLeftEdge(other);

	y2 = other->y + other->box.y;

	pushable = (self->flags & PUSHABLE);

	if ((self->flags & OBSTACLE) || depth == 1)
	{
		pushable = 0;
	}

	collided = FALSE;

	/* Test the vertical movement */

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(x1, y1, self->box.w, self->box.h, x2, y2 + ceil(other->dirY), other->box.w, other->box.h) == TRUE)
		{
			if (self->dirY < 0)
			{
				other->y -= self->dirY;

				other->dirY = self->dirY;

				dirX = other->dirX;

				other->dirX = 0;

				checkToMap(other);

				if (other->dirY == 0)
				{
					self->y = other->y + other->h;

					self->dirY = 0;
				}

				other->dirX = dirX;
			}

			/* Place the entity as close as possible */

			other->y = self->y + self->box.y;
			other->y -= other->box.h + other->box.y;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;

			collided = TRUE;
		}
	}

	else if (other->dirY < 0 && !(self->flags & ON_GROUND))
	{
		/* Trying to move up */

		if (collision(x1, y1, self->box.w, self->box.h, x2, y2 + floor(other->dirY), other->box.w, other->box.h) == TRUE)
		{
			/* Place the entity as close as possible */

			other->y = self->y + self->box.y;
			other->y += self->box.h;

			other->dirY = 0;

			collided = TRUE;
		}
	}

	/* Test the horizontal movement */

	if (other->dirX > 0 && collided == FALSE)
	{
		/* Trying to move right */

		if (collision(x1, y1, self->box.w, self->box.h, x2 + ceil(other->dirX), y2, other->box.w, other->box.h) == TRUE)
		{
			if (pushable != 0)
			{
				self->y -= self->dirY;

				/*self->dirX += ceil(other->dirX);*/

				self->dirX += other->dirX;

				wasOnGround = (self->flags & ON_GROUND);

				checkToMap(self);

				if (wasOnGround != 0)
				{
					self->flags |= ON_GROUND;
				}

				self->y -= self->dirY;

				depth++;

				if (checkEntityToEntity(self) != NULL)
				{
					self->x -= other->dirX;

					self->dirX = 0;
				}

				depth--;

				if (self->dirX == 0)
				{
					pushable = 0;
				}

				else
				{
					self->frameSpeed = 1;
				}

				self->y += self->dirY;
			}

			if (pushable == 0)
			{
				/* Place the entity as close as possible */

				other->x = getLeftEdge(self) - other->w;

				if (other->face == RIGHT)
				{
					other->x += other->w - other->box.x - other->box.w;
				}

				else
				{
					other->x += other->w - other->box.w;
				}

				other->dirX = (other->flags & BOUNCES) ? -other->dirX : 0;

				if ((other->flags & GRABBING) && other->target != NULL)
				{
					other->target->x -= other->target->dirX;
					other->target->dirX = 0;
				}
			}

			if ((other->flags & GRABBING) && other->target == NULL && (self->flags & PUSHABLE))
			{
				other->target = self;

				self->flags |= GRABBED;
			}

			collided = TRUE;
		}
	}

	else if (other->dirX < 0 && collided == FALSE)
	{
		/* Trying to move left */

		if (collision(x1, y1, self->box.w, self->box.h, x2 + floor(other->dirX), y2, other->box.w, other->box.h) == TRUE)
		{
			if (pushable != 0)
			{
				self->y -= self->dirY;

				/*self->dirX += floor(other->dirX);*/

				self->dirX += other->dirX;

				wasOnGround = (self->flags & ON_GROUND);

				checkToMap(self);

				if (wasOnGround != 0)
				{
					self->flags |= ON_GROUND;
				}

				self->y -= self->dirY;

				depth++;

				if (checkEntityToEntity(self) != NULL)
				{
					self->x -= other->dirX;

					self->dirX = 0;
				}

				depth--;

				if (self->dirX == 0)
				{
					pushable = 0;
				}

				else
				{
					self->frameSpeed = -1;
				}

				self->y += self->dirY;
			}

			if (pushable == 0)
			{
				/* Place the entity as close as possible */

				other->x = getRightEdge(self);

				if (other->face == RIGHT)
				{
					other->x -= other->box.x;
				}

				else
				{
					other->x -= other->w - (other->box.w + other->box.x);
				}

				other->dirX = (other->flags & BOUNCES) ? -other->dirX : 0;

				if ((other->flags & GRABBING) && other->target != NULL)
				{
					other->target->x -= other->target->dirX;
					other->target->dirX = 0;
				}
			}

			if ((other->flags & GRABBING) && other->target == NULL && (self->flags & PUSHABLE))
			{
				other->target = self;

				self->flags |= GRABBED;
			}

			collided = TRUE;
		}
	}
	/*
	else if (collided == FALSE)
	{
		if (self->x > other->x && depth == 1)
		{
			Place the entity as close as possible

			self->x = getRightEdge(other);

			self->dirX = 0;
		}

		else if (depth == 1)
		{
			Place the entity as close as possible

			self->x = getLeftEdge(other);

			self->x -= self->box.w;

			self->dirX = 0;
		}
	}
	*/
	other->x += other->dirX;
	other->y += other->dirY;
}

Entity *addEntity(Entity e, int x, int y)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == FALSE)
		{
			entity[i] = e;

			entity[i].currentFrame = 0;

			entity[i].inUse = TRUE;

			entity[i].x = x;

			entity[i].y = y;

			return &entity[i];
		}
	}

	showErrorAndExit("Could not add Entity %s", e.name);

	return NULL;
}

Entity *getEntityByName(char *name)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].name, name) == 0)
		{
			return &entity[i];
		}
	}

	return NULL;
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

Entity *getEntityByRequiredName(char *name)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].requires, name) == 0)
		{
			return &entity[i];
		}
	}

	return NULL;
}

EntityList *getEntitiesByObjectiveName(char *name)
{
	int i;
	EntityList *list;

	list = (EntityList *)malloc(sizeof(EntityList));

	if (list == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Entity List", (int)sizeof(EntityList));
	}

	list->next = NULL;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			addEntityToList(list, &entity[i]);
		}
	}

	return list;
}

EntityList *getEntitiesByRequiredName(char *name)
{
	int i;
	EntityList *list;

	list = (EntityList *)malloc(sizeof(EntityList));

	if (list == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Entity List", (int)sizeof(EntityList));
	}

	list->next = NULL;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].requires, name) == 0)
		{
			addEntityToList(list, &entity[i]);
		}
	}

	return list;
}

void freeEntityList(EntityList *list)
{
	EntityList *p, *q;

	if (list == NULL)
	{
		return;
	}

	for (p=list->next;p!=NULL;p=q)
	{
		q = p->next;

		free(p);
	}

	free(list);
}

Entity *getEntityByStartXY(int x, int y)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].startX == x && entity[i].startY == y)
		{
			return &entity[i];
		}
	}

	return NULL;
}

void activateEntitiesWithRequiredName(char *name, int active)
{
	int i;

	if (name == NULL || strlen(name) == 0)
	{
		showErrorAndExit("Activate Required Entities : Name is blank!");
	}

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].requires, name) == 0)
		{
			entity[i].active = active;
		}
	}
}

void activateEntitiesWithObjectiveName(char *name, int active)
{
	int i;

	if (name == NULL || strlen(name) == 0)
	{
		showErrorAndExit("Activate Objective Entities : Name is blank!");
	}

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			entity[i].active = active;
		}
	}
}

void activateEntitiesValueWithObjectiveName(char *name, int value)
{
	int i;
	Entity *temp;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].activate != NULL && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			temp = self;

			self = &entity[i];

			if (self->type == MANUAL_LIFT && self->active == FALSE)
			{
				setInfoBoxMessage(90, 255, 255, 255, _("This lift is not active"));
			}

			else
			{
				self->activate(value - self->health);
			}

			self = temp;
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
			if (collision(x, y, w, h, entity[i].x + entity[i].box.x, entity[i].y + entity[i].box.y, entity[i].box.w, entity[i].box.h) == 1)
			{
				e = self;

				self = &entity[i];

				self->activate(0);

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
		if (entity[i].inUse == TRUE && (entity[i].type == LINE_DEF || entity[i].type == SCRIPT_LINE_DEF))
		{
			self = &entity[i];

			self->flags &= ~NO_DRAW;

			self->action();
		}
	}
}

void changeDirection(Entity *other)
{
	self->dirX *= -1;

	checkToMap(self);

	self->face = self->face == RIGHT ? LEFT : RIGHT;
}

void writeEntitiesToFile(FILE *fp)
{
	int i, count;

	count = 0;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		self = &entity[i];

		if (self->inUse == TRUE && self->type != PROJECTILE && !(self->flags & DO_NOT_PERSIST))
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
			fprintf(fp, "DIR_X %0.2f\n", self->dirX);
			fprintf(fp, "DIR_Y %0.2f\n", self->dirY);
			fprintf(fp, "MAX_THINKTIME %d\n", self->maxThinkTime);
			fprintf(fp, "THINKTIME %d\n", self->thinkTime);
			fprintf(fp, "MENTAL %d\n", self->mental);
			fprintf(fp, "HEALTH %d\n", self->health);
			fprintf(fp, "DAMAGE %d\n", self->damage);
			fprintf(fp, "SPAWNTIME %d\n", self->spawnTime);
			fprintf(fp, "SPEED %0.2f\n", self->speed); /* Save the original speed, not the current speed */
			fprintf(fp, "WEIGHT %0.2f\n", self->originalWeight); /* Save the original weight, not the current weight */
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

	else if (strcmpignorecase(entityType, "ACTION_POINT") == 0)
	{
		e = addActionPoint("common/action_point", x, y);

		e->startX = x;
		e->startY = y;
		e->endX = x;
		e->endY = y;

		if (strcmpignorecase(objectiveName, " ") != 0)
		{
			STRNCPY(e->objectiveName, objectiveName, sizeof(e->objectiveName));
		}
	}

	else if (strcmpignorecase(entityType, "FALLING_PLATFORM") == 0)
	{
		addFallingPlatform(x, y, entityName);
	}

	else if (strcmpignorecase(entityType, "WEAK_WALL") == 0)
	{
		addWeakWall(entityName, x, y);
	}

	else
	{
		showErrorAndExit("ADD ENTITY command encountered unknown entity %s", entityType);
	}
}

void entityWalkTo(Entity *e, char *coords)
{
	int x, y, read;
	char wait[10], anim[10];

	read = sscanf(coords, "%d %d %s %s", &x, &y, wait, anim);

	e->targetX = x;
	e->targetY = y;

	if (!(e->flags & FLY))
	{
		e->targetY = e->y;
	}

	if (strcmpignorecase(wait, "WAIT") == 0)
	{
		e->action = &scriptEntityMoveToTarget;

		setScriptCounter(1);
	}

	else
	{
		e->action = &entityMoveToTarget;
	}

	e->face = (e->x < e->targetX) ? RIGHT : LEFT;

	if (read == 4)
	{
		setEntityAnimation(e, getAnimationTypeByName(anim));
	}

	else
	{
		setEntityAnimation(e, WALK);
	}

	if (e->type == PLAYER)
	{
		syncWeaponShieldToPlayer();
	}
}

void entityWalkToRelative(Entity *e, char *coords)
{
	int x, y, read;
	char wait[10], anim[10];

	read = sscanf(coords, "%d %d %s %s", &x, &y, wait, anim);

	e->targetX = e->x + x;
	e->targetY = e->y + y;

	if (!(e->flags & FLY))
	{
		e->targetY = e->y;
	}

	if (strcmpignorecase(wait, "WAIT") == 0)
	{
		e->action = &scriptEntityMoveToTarget;

		setScriptCounter(1);
	}

	else
	{
		e->action = &entityMoveToTarget;
	}

	e->face = (e->x < e->targetX) ? RIGHT : LEFT;

	if (read == 4)
	{
		setEntityAnimation(e, getAnimationTypeByName(anim));
	}

	else
	{
		setEntityAnimation(e, WALK);
	}

	if (e->type == PLAYER)
	{
		syncWeaponShieldToPlayer();
	}
}

static void scriptEntityMoveToTarget()
{
	if (self->speed == 0)
	{
		showErrorAndExit("%s has a speed of 0 and will not move!", self->objectiveName);
	}

	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;
	}

	if (!(self->flags & FLY))
	{
		self->targetY = self->y;
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

		if (self->type == PLAYER)
		{
			self->action = &playerWaitForDialog;

			syncWeaponShieldToPlayer();
		}

		else
		{
			self->dirX = 0;
			self->dirY = 0;

			self->action = &scriptDoNothing;
		}

		setScriptCounter(-1);
	}

	else
	{
		checkToMap(self);
	}
}

static void scriptDoNothing()
{
	if (self->standingOn != NULL)
	{
		self->dirX = self->standingOn->dirX;

		if (self->standingOn->dirY > 0)
		{
			self->dirY = self->standingOn->dirY + 1;
		}

		self->flags |= ON_GROUND;
	}

	checkToMap(self);
}

static void entityMoveToTarget()
{
	if (self->speed == 0)
	{
		showErrorAndExit("%s has a speed of 0 and will not move!", self->objectiveName);
	}

	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;

		self->dirX = 0;
	}

	if (!(self->flags & FLY))
	{
		self->targetY = self->y;
	}

	if (abs(self->y - self->targetY) > self->speed)
	{
		self->dirY = (self->y < self->targetY ? self->speed : -self->speed);
	}

	else
	{
		self->y = self->targetY;

		self->dirY = 0;
	}

	if (self->x == self->targetX && self->y == self->targetY)
	{
		setEntityAnimation(self, STAND);

		if (self->type == PLAYER)
		{
			self->action = &playerWaitForDialog;

			syncWeaponShieldToPlayer();
		}
	}

	checkToMap(self);
}

void rotateAroundStartPoint()
{
	float x, y, radians;

	x = self->endX;
	y = self->endY;

	self->thinkTime += self->speed;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	radians = DEG_TO_RAD(self->thinkTime);

	self->x = (x * cos(radians) - y * sin(radians));
	self->y = (x * sin(radians) + y * cos(radians));

	self->x += self->startX - (self->w / 2);
	self->y += self->startY - (self->h / 2);
}

int countSiblings(Entity *sibling, int *total)
{
	int i, remaining = 0;

	*total = 0;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && sibling != &entity[i] && sibling->type == entity[i].type
			&& strcmpignorecase(self->objectiveName, entity[i].objectiveName) == 0)
		{
			if (entity[i].active == FALSE)
			{
				remaining++;
			}

			(*total)++;
		}
	}

	return remaining;
}

void doTeleport()
{
	int i;
	float speed;
	Entity *e;

	if (abs(self->x - self->targetX) < TELEPORT_SPEED && abs(self->y - self->targetY) < TELEPORT_SPEED)
	{
		self->flags &= ~(NO_DRAW|HELPLESS|TELEPORTING);

		self->x = self->targetX;
		self->y = self->targetY;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->dirY = self->dirX = 0;

		self->standingOn = NULL;

		if (!(self->flags & NO_END_TELEPORT_SOUND))
		{
			playSoundToMap("sound/common/teleport.ogg", -1, self->x, self->y, 0);
		}
	}

	else
	{
		self->flags |= NO_DRAW|HELPLESS|INVULNERABLE;

		speed = getDistance(self->x, self->y, self->targetX, self->targetY) / 20;

		speed = speed < TELEPORT_SPEED ? TELEPORT_SPEED : (speed > 30 ? 30 : speed);

		normalize(&self->dirX, &self->dirY);

		self->dirX *= speed;
		self->dirY *= speed;

		self->x += self->dirX;
		self->y += self->dirY;

		for (i=0;i<5;i++)
		{
			e = addBasicDecoration(self->x, self->y, "decoration/particle");

			if (e != NULL)
			{
				e->x += prand() % self->w;
				e->y += prand() % self->h;

				e->thinkTime = 5 + prand() % 30;

				setEntityAnimation(e, prand() % 5);
			}
		}
	}
}

int getLeftEdge(Entity *e)
{
	return e->face == RIGHT ? e->x + e->box.x : e->x + e->w - e->box.w - e->box.x;
}

int getRightEdge(Entity *e)
{
	return e->face == RIGHT ? e->x + e->box.x + e->box.w : e->x + e->w - e->box.x;
}

void addEntityToList(EntityList *head, Entity *e)
{
	EntityList *listHead, *list;

	listHead = head;

	while (listHead->next != NULL)
	{
		listHead = listHead->next;
	}

	list = (EntityList *)malloc(sizeof(EntityList));

	list->entity = e;
	list->next = NULL;

	listHead->next = list;
}

void killEntity(char *name)
{
	Entity *e = getEntityByObjectiveName(name);

	if (e != NULL)
	{
		e->inUse = FALSE;
	}
}

int atTarget()
{
	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		return TRUE;
	}

	return FALSE;
}

void faceTarget()
{
	self->face = self->target->x < self->x ? LEFT : RIGHT;
}

void addToDrawLayer(Entity *e, int layer)
{
	drawLayer[layer][drawLayerIndex[layer]] = e;

	drawLayerIndex[layer]++;
}

void clearDrawLayers()
{
	int i;

	for (i=0;i<MAX_LAYERS;i++)
	{
		drawLayerIndex[i] = 0;

		memset(drawLayer[i], 0, sizeof(Entity *) * MAX_ENTITIES);
	}
}

void teleportEntityFromScript(Entity *e, char *line)
{
	sscanf(line, "%d %d", &e->targetX, &e->targetY);

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	playSoundToMap("sound/common/teleport.ogg", -1, e->x, e->y, 0);
}
