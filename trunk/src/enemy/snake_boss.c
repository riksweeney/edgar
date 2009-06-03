/*
Copyright (C) 2009 Parallel Realities

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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../item/key_items.h"

extern Entity *self;

static void bodyWait(void);
static void initialise(void);
static void headWait(void);
static void riseUp(void);
static void touch(Entity *);
static void takeDamage(Entity *, int);
static void die(void);

Entity *addSnakeBoss(int x, int y, char *name)
{
	char bodyName[MAX_VALUE_LENGTH];
	int i;
	Entity *head, *body[8];

	snprintf(bodyName, sizeof(bodyName), "%s_body", name);

	/* Create in reverse order so that it is draw correctly */

	for (i=7;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			printf("No free slots to add a Snake Boss body part\n");

			exit(1);
		}

		loadProperties(bodyName, body[i]);

		body[i]->x = x;
		body[i]->y = y;

		body[i]->action = &bodyWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &touch;
		body[i]->die = &entityDie;
		body[i]->takeDamage = NULL;

		body[i]->type = ENEMY;

		body[i]->active = FALSE;

		setEntityAnimation(body[i], STAND);
	}

	head = getFreeEntity();

	if (head == NULL)
	{
		printf("No free slots to add a Snake Boss\n");

		exit(1);
	}

	loadProperties(name, head);

	head->x = x;
	head->y = y;

	head->action = &initialise;

	head->draw = &drawLoopingAnimationToMap;
	head->touch = &touch;
	head->die = &die;
	head->takeDamage = &takeDamage;

	head->type = ENEMY;

	head->active = FALSE;

	setEntityAnimation(head, STAND);

	/* Link the sections */

	for (i=7;i>=0;i--)
	{
		if (i == 0)
		{
			head->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];
		}
	}

	return head;
}

static void bodyWait()
{

}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();

	if (self->active == TRUE)
	{
		/*
		adjustMusicVolume(-1);

		centerMapOnEntity(NULL);

		if (minX < self->endX)
		{
			minX++;
		}

		else if (minX > self->endX)
		{
			minX--;
		}

		if (minY < self->endY)
		{
			minY++;
		}

		else if (minY > self->endY)
		{
			minY--;
		}

		setMapStartX(minX);
		setMapStartY(minY);

		setCameraPosition(minX, minY);

		if (minX == self->endX && minY == self->endY)
		{
			self->targetX = self->startX;
			self->targetY = self->startY - 128;

			setEntityAnimation(self, STAND);

			self->action = &riseUp;
		}
		*/
		self->targetX = self->startX;
		self->targetY = self->startY - 128;

		setEntityAnimation(self, STAND);

		self->action = &riseUp;
	}
}

static void headWait()
{
	int x;
	Entity *e;

	/* Sway back and forth */

	self->dirX += 0.5;

	if (self->dirX >= 360)
	{
		self->dirX = 0;
	}

	x = 32;

	self->x = self->targetX + (sin(DEG_TO_RAD(self->dirX)) * x);

	e = self->target;

	while (e != NULL)
	{
		x /= 1.5;
		
		if (e->target != NULL)
		{
			e->x = self->targetX;
			
			e->x += (sin(DEG_TO_RAD(self->dirX)) * x);
		}

		e = e->target;
	}
}

static void riseUp()
{
	float y = 0.5;
	Entity *e;

	self->y -= y;

	e = self->target;

	while (e != NULL)
	{
		y /= 1.5;
		
		if (e->target != NULL)
		{
			e->y -= y;
		}
		
		e = e->target;
	}

	if (self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->action = &headWait;
	}
}

static void touch(Entity *other)
{
	Entity *temp;

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, 1);
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

	else if (other->type == PLAYER)
	{
		temp = self;

		self = other;

		self->takeDamage(other, other->damage);

		self = temp;
	}
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);
		}

		else
		{
			self->thinkTime = 180;

			self->flags &= ~FLY;

			setEntityAnimation(self, STAND);

			self->frameSpeed = 0;

			self->takeDamage = NULL;
			self->touch = NULL;

			self->action = &die;
		}
	}
}

static void die()
{
	Entity *e;

	self->thinkTime--;

	if (!(self->flags & ON_GROUND))
	{
		if (self->thinkTime == 0)
		{
			self->dirX = (self->face == LEFT ? -6 : 6);
		}

		else if (self->thinkTime < 0)
		{
			self->dirX += (self->face == LEFT ? 0.5 : 0.5);
		}

		checkToMap(self);

		if (self->flags & ON_GROUND)
		{
			self->dirX = 0;

			shakeScreen(MEDIUM, 90);

			self->thinkTime = 120;
		}
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

			e->dirY = ITEM_JUMP_HEIGHT;

			self->action = &entityDieNoDrop;

			e = self->target;

			while (e != NULL)
			{
				e->action = &entityDieNoDrop;

				e = e->target;
			}
		}
	}
}
