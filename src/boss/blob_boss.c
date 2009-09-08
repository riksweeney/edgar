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
#include "../custom_actions.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../audio/audio.h"
#include "../graphics/gib.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../hud.h"

extern Entity *self, player;

static void wait(void);
static void initialise(void);
static void takeDamage(Entity *, int);
static void attackFinished(void);
static void doIntro(void);
static void introPause(void);

Entity *addBlobBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Blob Boss\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();

	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			setEntityAnimation(self, STAND);

			self->action = &doIntro;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			self->touch = &entityTouch;

			self->endX = self->damage;

			self->damage = 0;

			initBossHealthBar();

			playBossMusic();
		}
	}
}

static void doIntro()
{
	long onGround = (self->flags & ON_GROUND);

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->face = LEFT;

		setEntityAnimation(self, CUSTOM_1);

		self->thinkTime = 60;

		self->animationCallback = &introPause;
	}
}

static void introPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->touch = &entityTouch;
		self->takeDamage = &takeDamage;

		self->damage = self->endX;

		attackFinished();
	}

	checkToMap(self);
}

static void wait()
{
	self->dirX = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->flags &= ~INVULNERABLE;

	setEntityAnimation(self, STAND);

	self->frameSpeed = 1;

	self->thinkTime = 90;

	self->action = &wait;
}

static void takeDamage(Entity *other, int damage)
{
	int i;

	if (!(self->flags & INVULNERABLE))
	{
		if (other->element == LIGHTNING)
		{
			self->health -= damage;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			i = prand() % 3;

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
	}
}

static void activate(int val)
{
	Entity *e, *temp;

	e = getInventoryItem(_("Tesla Pack"));

	if (e != NULL)
	{
		temp = self;

		self = e;

		self->target = e;

		self->activate(val);

		self = temp;
	}
}

static void stunnedTouch(Entity *other)
{
	Entity *e, *temp;

	e = getInventoryItem(_("Tesla Pack"));

	if (e != NULL)
	{
		setInfoBoxMessage(5,  _("Press Action to attach the Tesla Pack"));
	}
}

static void splitAttackInit()
{
	int i;
	Entity *e;

	self->maxThinkTime = 0;

	for (i=0;i<30;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			printf("No free slots to add a Blob Boss part\n");

			exit(1);
		}

		loadProperties(self->name, e);

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->action = &scatterAndAttack;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &partTakeDamage;

		e->head = self;

		e->type = ENEMY;

		self->maxThinkTime++;

		setEntityAnimation(e, STAND);
	}
}

static void partTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			i = prand() % 3;

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

		else
		{
			self->head->maxThinkTime--;

			self->action = &partWait;

			self->touch = NULL;
		}
	}
}

static void partAttack()
{

}

static void partWait()
{
	checkToMap(self);
}
