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
#include "../enemy/rock.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../hud.h"
#include "../map.h"
#include "../player.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../system/error.h"

extern Entity *self, player;
extern Game game;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void wait(void);
static void hover(void);
static void attackFinished(void);
static void knifeThrowInit(void);
static void knifeWait(void);
static void knifeAttack(void);
static void knifeBlock(void);
static void knifeBlockWait(void);
static void takeDamage(Entity *, int);
static void knifeThrow(void);
static void ceilingDropInit(void);
static void ceilingDropWait(void);
static void knifeDie(void);
static void verticalKnifeThrowInit(void);
static void verticalKnifeWait(void);
static void lavaCeilingDropWait(void);
static void lavaCeilingDropInit(void);
static void lavaCeilingDropEnd(void);
static void stunned(void);
static void stunFinish(void);

Entity *addMataeus(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Mataeus");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;
	e->pain = &enemyPain;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	self->thinkTime = 180;

	setEntityAnimation(self, CUSTOM_1);

	self->action = &doIntro;

	self->startY = self->y;
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, CUSTOM_2);

		self->animationCallback = &introPause;
	}
}

static void introPause()
{
	self->dirX = 0.5;

	checkToMap(self);

	playBossMusic();

	initBossHealthBar();

	self->action = &attackFinished;

	self->thinkTime = 90;

	facePlayer();

	self->startY = self->y;
}

static void attackFinished()
{
	self->frameSpeed = 1;

	self->dirX = 0;

	self->thinkTime = 90;

	self->damage = 1;

	self->mental = 0;

	self->action = &wait;

	self->touch = &entityTouch;

	self->takeDamage = &takeDamage;

	setEntityAnimation(self, STAND);

	hover();
}

static void wait()
{
	int i;

	self->dirX = 0;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->health >= 1500)
		{
			i = prand() % 2;

			switch (i)
			{
				case 0:
					self->action = &ceilingDropInit;
				break;

				default:
					self->action = &knifeThrowInit;
				break;
			}
		}

		else if (self->health >= 1000)
		{
			i = prand() % 2;

			switch (i)
			{
				case 0:
					self->action = &lavaCeilingDropInit;
				break;

				default:
					self->action = &verticalKnifeThrowInit;
				break;
			}
		}
	}

	hover();
}

static void hover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;
}

static void knifeThrowInit()
{
	int i, radians;
	Entity *e;

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}

		radians = prand() % 7;

		if (radians == 0)
		{
			loadProperties("boss/mataeus_knife_special", e);

			e->reactToBlock = &knifeBlock;
		}

		else
		{
			loadProperties("boss/mataeus_knife", e);

			e->reactToBlock = &bounceOffShield;
		}

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		radians = DEG_TO_RAD(i * 60);

		e->x += (0 * cos(radians) - 0 * sin(radians));
		e->y += (0 * sin(radians) + 0 * cos(radians));

		e->action = &knifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		e->health = radians;

		e->head = self;

		setEntityAnimation(e, WALK);
	}

	self->mental = 0;

	playSoundToMap("sound/boss/mataeus/create_knife.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 60;

	self->action = &knifeThrow;
}

static void verticalKnifeThrowInit()
{
	int i, startX, startY;
	Entity *e;

	startX = getMapStartX();
	startY = getMapStartY() + 64;

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}

		loadProperties("boss/mataeus_knife", e);

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->targetX = startX + 64 + (i * 96);
		e->targetY = startY;

		e->action = &verticalKnifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 30;
		e->dirY *= 30;

		e->head = self;

		setEntityAnimation(e, WALK);
	}

	self->mental = 0;

	playSoundToMap("sound/boss/mataeus/create_knife.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 90;

	self->action = &knifeThrow;
}

static void knifeThrow()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			playSoundToMap("sound/boss/mataeus/throw_knife.ogg", BOSS_CHANNEL, self->x, self->y, 0);

			self->mental = 1;
		}

		else
		{
			self->action = &attackFinished;
		}

		self->thinkTime = 120;
	}

	hover();
}

static void verticalKnifeWait()
{
	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		if (self->head->mental == 1)
		{
			self->thinkTime = 30;

			self->action = &knifeAttack;

			self->dirX = 0;
			self->dirY = 16;
			
			self->startX = self->dirX;
			self->startY = self->dirY;

			self->thinkTime = 60;

			setEntityAnimation(self, ATTACK_2);

			self->health = 1;
		}
	}

	checkToMap(self);
}

static void knifeWait()
{
	float startX, startY, endX, endY;

	self->mental += 4;

	if (self->mental >= 64)
	{
		self->mental = 64;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	self->x += (self->mental * cos(self->health) - 0 * sin(self->health));
	self->y += (self->mental * sin(self->health) + 0 * cos(self->health));

	if (self->head->mental == 1)
	{
		self->thinkTime = 30;

		self->action = &knifeAttack;

		startX = self->x;
		startY = self->y;

		endX = player.x + player.w / 2;
		endY = player.y;

		calculatePath(startX, startY, endX, endY, &self->dirX, &self->dirY);

		self->dirX *= 30;
		self->dirY *= 30;
		
		self->startX = self->dirX;
		self->startY = self->dirY;

		self->thinkTime = 60;

		facePlayer();

		setEntityAnimation(self, ATTACK_1);

		self->health = 1;
	}
}

static void knifeAttack()
{
	checkToMap(self);

	if (self->dirX != self->startX || self->dirY == self->startY)
	{
		self->damage = 0;

		self->flags |= FLY;

		self->dirX = 0;

		self->dirY = 0;

		self->thinkTime = 15;

		self->action = &knifeDie;
	}
}

static void knifeDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void knifeBlock()
{
	self->flags &= ~FLY;

	self->dirX = self->x < player.x ? -5 : 5;

	self->dirY = -5;

	self->thinkTime = 120;

	self->damage = 50;

	self->action = &knifeBlockWait;

	self->activate = &throwItem;
}

static void knifeBlockWait()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;

		self->dirX = 0;

		self->action = &generalItemAction;

		self->touch = &keyItemTouch;

		self->activate = &throwItem;
	}
}

static void ceilingDropInit()
{
	EntityList *list = getEntitiesByObjectiveName("MATAEUS_CEILING");
	EntityList *l;
	Entity *e;
	int i;

	i = 0;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		if (e->mental == 0)
		{
			e->thinkTime = prand() % 1200;

			e->mental = 1;
		}
	}

	freeEntityList(list);

	self->thinkTime = 60;

	self->action = &ceilingDropWait;

	hover();
}

static void ceilingDropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void lavaCeilingDropInit()
{
	EntityList *list = getEntitiesByObjectiveName("MATAEUS_CEILING");
	EntityList *l;
	Entity *e;
	int i;

	i = 0;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		if (e->mental == 0)
		{
			e->thinkTime = prand() % 300;

			e->mental = 2;
		}
	}

	freeEntityList(list);

	self->thinkTime = 1200;

	self->action = &lavaCeilingDropWait;

	hover();
}

static void lavaCeilingDropWait()
{
	EntityList *list;
	EntityList *l;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		list = getEntitiesByObjectiveName("MATAEUS_CEILING");

		for (l=list->next;l!=NULL;l=l->next)
		{
			e = l->entity;

			e->mental = 3;
		}

		self->thinkTime = 300;

		self->action = &lavaCeilingDropEnd;
	}

	hover();
}

static void lavaCeilingDropEnd()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void takeDamage(Entity *other, int damage)
{
	int health;
	
	if (self->flags & HELPLESS)
	{
		entityTakeDamageNoFlinch(other, damage);
	}
	
	else
	{
		if (strcmpignorecase(other->name, "boss/mataeus_knife_special") != 0)
		{
			playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

			if (prand() % 10 == 0)
			{
				setInfoBoxMessage(60, _("This weapon is not having any effect..."));
			}

			setCustomAction(self, &invulnerableNoFlash, 20, 0);
		}

		else
		{
			health = self->health;

			self->health -= damage;

			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			other->inUse = FALSE;

			enemyPain();

			if ((health >= 1500 && self->health < 1500) || (health >= 1000 && self->health < 1000))
			{
				self->flags &= ~FLY;

				self->action = &stunned;
			}
		}
	}
}

static void stunned()
{
	int i;
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		setCustomAction(self, &helpless, 300, 0);

		if (onGround == 0)
		{
			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		}

		self->action = &stunFinish;
	}
}

static void stunFinish()
{
	self->flags |= FLY;

	self->dirY = -6;

	self->startX = 0;

	checkToMap(self);

	if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->action = &attackFinished;
	}
}
