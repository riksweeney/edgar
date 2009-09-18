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
#include "../inventory.h"
#include "../world/target.h"
#include "../player.h"

extern Entity *self, player;

static void wait(void);
static void initialise(void);
static void takeDamage(Entity *, int);
static void attackFinished(void);
static void doIntro(void);
static void introPause(void);
static void floatInContainer(void);
static void stunnedTouch(Entity *);
static void splitAttackInit(void);
static void partWait(void);
static void partAttack(void);
static void activate(int);
static void leaveFinish(void);
static void reform(void);
static void headWait(void);
static void headReform(void);
static void partDie(void);
static void partGrab(Entity *);
static void stickToPlayerAndDrain(void);
static void fallOff(void);
static void bounceAroundInit(void);
static void bounceAround(void);
static void punchAttackInit(void);
static void punchSink(void);
static void lookForPlayer(void);
static void punch(void);
static void punchFinish(void);
static void eatInit(void);
static void eatAttack(void);
static void eat(void);
static void eatExplode(void);
static void explodeWait(void);
static void eatTakeDamage(Entity *, int);
static void shudder(void);
static Target *getTarget(void);
static void grubAttackInit(void);
static void grubAttackWait(void);
static void spinAttackStart(void);
static void spinAttack(void);
static void spinAttackEnd(void);
static void grubAttackFinish(void);

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

	if (strcmpignorecase("boss/blob_boss_1", name) == 0)
	{
		e->action = &floatInContainer;
		e->takeDamage = NULL;
	}

	else
	{
		e->action = &initialise;
		e->touch = NULL;

		e->flags &= ~FLY;
	}

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

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

			self->startX = self->maxThinkTime = 60;

			self->thinkTime = 6;

			self->flags &= ~FLY;

			self->action = &doIntro;

			setEntityAnimation(self, WALK);

			self->frameSpeed = 0;
		}
	}
}

static void doIntro()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			printf("No free slots to add a Blob Boss Part\n");

			exit(1);
		}

		loadProperties("boss/blob_boss_part", e);

		setEntityAnimation(e, STAND);

		e->draw = &drawLoopingAnimationToMap;

		e->type = ENEMY;

		e->damage = 0;

		e->head = self;

		e->action = &reform;

		e->x = self->x + self->w / 2 - e->w / 2;

		e->y = self->startY + self->h / 2 - e->h / 2;

		e->dirX = (10 + prand() % 50) * (prand() % 2 == 0 ? 1 : -1);

		e->dirX /= 10;

		e->thinkTime = 120;

		e->targetX = self->x + self->w / 2;

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &introPause;
		}

		self->thinkTime = 6;
	}

	checkToMap(self);
}

static void reform()
{
	checkToMap(self);

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->flags & ON_GROUND)
		{
			self->dirX = 0;
		}
	}

	else
	{
		if (fabs(self->targetX - self->x) <= fabs(self->dirX))
		{
			self->head->startX--;

			self->head->flags &= ~NO_DRAW;

			self->inUse = FALSE;

			if (((int)self->head->startX) % 10 == 0)
			{
				self->head->currentFrame++;
			}

			playSoundToMap("sound/boss/blob_boss/plop.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		}

		if (self->flags & ON_GROUND)
		{
			self->dirX = (self->x < self->targetX ? self->speed : -self->speed);

			self->dirY = -6;
		}
	}
}

static void introPause()
{
	checkToMap(self);

	if (self->startX <= 0)
	{
		self->takeDamage = &takeDamage;

		self->action = &attackFinished;

		playBossMusic();

		initBossHealthBar();

		self->touch = &entityTouch;
		
		self->mental = 15;
	}
}

static void wait()
{
	int i;

	self->dirX = 0;

	facePlayer();

	self->thinkTime--;
	
	setEntityAnimation(self, (self->mental <= 0 || self->health <= 300) ? JUMP : STAND);

	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->mental <= 0)
		{
			self->action = &eatInit;
		}

		else if (self->health > 3000)
		{
			self->action = &bounceAroundInit;
		}
		
		else if (self->health > 2000)
		{
			self->action = prand() % 2 == 0 ? &bounceAroundInit : &punchAttackInit;
		}
		
		else if (self->health > 1000)
		{
			i = prand() % 3;
			
			switch (i)
			{
				case 0:
					self->action = &punchAttackInit;
				break;

				case 1:
					self->action = &bounceAroundInit;
				break;

				default:
					self->action = &grubAttackInit;
				break;
			}
		}
		
		else
		{
			i = prand() % 2;
			
			switch (i)
			{
				case 0:
					self->action = &splitAttackInit;
				break;
				
				case 1:
					self->action = &punchAttackInit;
				break;
			}
		}
	}

	checkToMap(self);
}

static void grubAttackInit()
{
	facePlayer();

	setEntityAnimation(self, ATTACK_2);

	self->thinkTime = 30;

	self->animationCallback = &grubAttackWait;

	self->maxThinkTime = 1 + prand() % 5;
}

static void grubAttackWait()
{
	setEntityAnimation(self, ATTACK_3);
	
	self->thinkTime--;

	self->action = &grubAttackWait;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, ATTACK_4);

		self->action = &spinAttackStart;

		self->thinkTime = 1;
	}
}

static void spinAttackStart()
{
	self->flags |= INVULNERABLE;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->frameSpeed = 2;

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		self->speed = self->originalSpeed * 6;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &spinAttack;

		self->thinkTime = 180;

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void spinAttack()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self))
	{
		shakeScreen(MEDIUM, 15);

		self->dirX = self->face == LEFT ? 3 : -3;

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;

		playSoundToMap("sound/common/crash.ogg", -1, self->x, self->y, 0);

		facePlayer();
	}

	else if (self->thinkTime <= 0)
	{
		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}
}

static void spinAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		facePlayer();

		self->dirX = 0;

		self->maxThinkTime--;
		
		if (self->maxThinkTime > 0)
		{
			self->action = &spinAttackStart;
			
			self->thinkTime = 0;
		}
		
		else
		{
			self->action = &grubAttackFinish;
			
			self->thinkTime = 30;
		}
	}
}

static void grubAttackFinish()
{
	if (self->frameSpeed > 0)
	{
		self->frameSpeed = -1;
		
		facePlayer();
	
		setEntityAnimation(self, ATTACK_2);
		
		self->animationCallback = &attackFinished;
		
		self->frameSpeed = 0;
	}
	
	else if (self->thinkTime > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->frameSpeed = -1;
		}
	}
}

static void eatInit()
{
	self->damage = 0;

	self->layer = FOREGROUND_LAYER;

	facePlayer();

	self->action = &eatAttack;

	self->takeDamage = &eatTakeDamage;

	self->mental = 10000;
}

static void eatAttack()
{
	int bossMid, playerMid;

	bossMid = self->x + self->w / 2;

	playerMid = player.x + player.w / 2;

	if (abs(bossMid - playerMid) < 4)
	{
		self->dirX = 0;

		self->thinkTime = 300;

		self->target = &player;

		self->action = &eat;

		self->mental = 10;
	}

	else
	{
		self->dirX = bossMid < playerMid ? self->speed : -self->speed;
	}

	checkToMap(self);
}

static void eat()
{
	Entity *temp;

	self->thinkTime--;

	self->target->x = self->x + self->w / 2 - self->target->w / 2;

	self->target->y = self->y + self->h / 2 - self->target->h / 2;

	self->target->y += cos(DEG_TO_RAD(self->thinkTime)) * 8;

	if (self->thinkTime <= 0)
	{
		temp = self;

		self = self->target;

		playerGib();

		self = temp;

		self->action = &wait;

		self->maxThinkTime = 0;
	}
}

static void eatExplode()
{
	int i;
	Entity *e;
	Target *t;

	self->maxThinkTime = 0;

	t = getTarget();
	
	self->startX = 0;

	for (i=0;i<60;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			printf("No free slots to add a Blob Boss part\n");

			exit(1);
		}

		loadProperties("boss/blob_boss_part", e);

		setEntityAnimation(e, STAND);

		e->x = self->x;
		e->y = self->y;

		e->x += prand() % self->w;

		e->y += prand() % (self->h - e->h);

		e->dirX = (10 + prand() % 20) * (prand() % 2 == 0 ? 1 : -1);

		e->dirX /= 10;

		e->dirY = -6 - prand() % 4;

		e->action = &partWait;

		e->pain = &enemyPain;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->type = ENEMY;

		e->targetX = t->x;
		
		self->startX++;
	}

	self->target = NULL;

	self->thinkTime = 120;

	self->action = &explodeWait;

	setEntityAnimation(self, WALK);

	self->flags |= NO_DRAW;

	self->frameSpeed = 0;

	self->touch = NULL;
}

static void bounceAroundInit()
{
	self->maxThinkTime = 8;

	self->touch = &entityTouch;

	self->action = &bounceAround;

	self->dirY = -16;

	self->face = self->face == LEFT ? RIGHT : LEFT;
}

static void bounceAround()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->maxThinkTime--;

		if (self->maxThinkTime > 0)
		{
			self->dirY = -16;
		}

		else
		{
			self->action = &attackFinished;
		}
	}

	if (self->dirX == 0 && self->maxThinkTime != 8)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
}

static void punchAttackInit()
{
	self->targetY = self->y + self->h;

	self->maxThinkTime = 3 + prand() % 3;

	self->layer = BACKGROUND_LAYER;

	self->action = &punchSink;
}

static void punchSink()
{
	Target *t;

	if (self->y < self->targetY)
	{
		self->y += 3;
	}

	else
	{
		self->y = self->targetY;

		setEntityAnimation(self, ATTACK_1);

		if (self->maxThinkTime > 0 && player.health > 0)
		{
			self->action = &lookForPlayer;

			self->dirX = self->speed * 1.5;
		}

		else
		{
			self->action = &punchFinish;

			t = getTarget();

			self->targetX = t->x;

			self->targetY = self->y - self->h;

			self->dirX = self->speed;
		}
	}
}

static void lookForPlayer()
{
	float target = player.x - self->w / 2 + player.w / 2;

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 30;

		self->action = &punch;
	}

	else
	{
		self->x += target > self->x ? self->dirX : -self->dirX;
	}
}

static void punch()
{
	Entity *e;

	if (self->y > self->targetY)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->thinkTime == 0)
			{
				playSoundToMap("sound/common/crumble.ogg", BOSS_CHANNEL, self->x, self->y, 0);

				shakeScreen(MEDIUM, 15);
			}

			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = -3;
			e->dirY = -8;

			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = 3;
			e->dirY = -8;

			self->y -= 12;

			if (self->y <= self->targetY)
			{
				self->y = self->targetY;

				self->maxThinkTime--;

				self->thinkTime = self->maxThinkTime > 0 ? 30 : 90;
			}
		}
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime < 0)
		{
			self->targetY = self->y + self->h;

			self->action = &punchSink;
		}
	}

	facePlayer();
}

static void punchFinish()
{
	if (fabs(self->x - self->targetX) <= fabs(self->dirX))
	{
		setEntityAnimation(self, STAND);

		if (self->y > self->targetY)
		{
			self->y -= 2;
		}

		else
		{
			self->action = &attackFinished;
		}
	}

	else
	{
		self->x += self->x < self->targetX ? self->dirX : -self->dirX;
	}

	facePlayer();
}

static void attackFinished()
{
	self->flags &= ~INVULNERABLE;

	self->layer = MID_GROUND_LAYER;

	setEntityAnimation(self, (self->mental <= 0 || self->health <= 300) ? JUMP : STAND);

	self->speed = self->originalSpeed;

	self->dirX = 0;

	self->frameSpeed = 1;

	self->thinkTime = 90;

	self->damage = 1;

	self->action = &wait;

	self->touch = &entityTouch;

	self->activate = NULL;
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (other->element == LIGHTNING)
		{
			self->health -= damage;

			self->thinkTime = 120;

			self->startX = self->x;

			self->action = &shudder;
		}

		else if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			self->mental--;

			enemyPain();
		}
	}
}

static void eatTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		setCustomAction(self, &flashWhite, 6, 0);
		setCustomAction(self, &invulnerableNoFlash, 20, 0);

		self->mental--;

		enemyPain();

		if (self->mental <= 0)
		{
			self->action = &eatExplode;
		}
	}
}

static void activate(int val)
{
	Entity *e, *temp;

	if (!(self->flags & NO_DRAW))
	{
		e = getInventoryItem(_("Tesla Pack"));

		printf("Getting pack\n");

		if (e != NULL && e->health != 0)
		{
			printf("Using pack\n");

			temp = self;

			self = e;

			self->target = temp;

			self->activate(val);

			self = temp;
		}

		else
		{
			printf("Could not find pack\n");
		}
	}
}

static void stunnedTouch(Entity *other)
{
	Entity *e;

	if (!(self->flags & NO_DRAW))
	{
		e = getInventoryItem(_("Tesla Pack"));

		if (e != NULL && e->health != 0)
		{
			setInfoBoxMessage(5,  _("Press Action to attach the Tesla Pack"));
		}
	}
}

static void splitAttackInit()
{
	int i;
	Entity *e;
	Target *t;

	self->maxThinkTime = 0;

	t = getTarget();

	for (i=0;i<60;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			printf("No free slots to add a Blob Boss part\n");

			exit(1);
		}

		loadProperties("boss/blob_boss_part", e);

		setEntityAnimation(e, WALK);

		e->x = self->x;
		e->y = self->y;

		e->x += prand() % self->w;

		e->y += prand() % (self->h - e->h);

		e->dirX = (10 + prand() % 20) * (prand() % 2 == 0 ? 1 : -1);

		e->dirX /= 10;

		e->dirY = -6;

		e->action = &partAttack;

		e->touch = &partGrab;

		e->die = &partDie;

		e->pain = &enemyPain;

		e->draw = &drawLoopingAnimationToMap;

		e->takeDamage = &entityTakeDamageNoFlinch;

		e->head = self;

		e->type = ENEMY;

		e->thinkTime = 60;

		e->targetX = t->x;
	}

	self->maxThinkTime = 60;

	self->action = &headWait;

	setEntityAnimation(self, WALK);

	self->flags |= NO_DRAW;

	self->frameSpeed = 0;

	self->touch = NULL;
}

static void partDie()
{
	self->head->maxThinkTime--;

	setEntityAnimation(self, STAND);

	printf("%d to go\n", self->head->maxThinkTime);

	self->action = &partWait;

	self->touch = NULL;

	self->takeDamage = NULL;
}

static void partAttack()
{
	long onGround = (self->flags & ON_GROUND);

	checkToMap(self);

	/* Bounce towards player */

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		if (onGround == 0)
		{
			self->thinkTime = prand() % 60;
		}

		else
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->dirX = (self->x < player.x ? self->speed : -self->speed);

				self->dirY = -6;
			}
		}
	}
}

static void partGrab(Entity *other)
{
	if (self->health <= 0)
	{
		return;
	}

	if (other->type == WEAPON && (other->flags & ATTACKING))
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

	else if (other->type == PLAYER && !(self->flags & GRABBING))
	{
		self->startX = (prand() % (other->w / 2)) * (prand() % 2 == 0 ? 1 : -1);

		self->startY = prand() % (other->h - self->h);

		setCustomAction(other, &slowDown, 3, 1);

		self->action = &stickToPlayerAndDrain;

		self->touch = NULL;

		self->flags |= GRABBING;

		self->layer = FOREGROUND_LAYER;

		other->flags |= GRABBED;

		self->thinkTime = 0;

		self->mental = 3 + (prand() % 3);
	}
}

static void stickToPlayerAndDrain()
{
	Entity *temp;

	setCustomAction(&player, &slowDown, 3, 0);

	self->x = player.x + (player.w - self->w) / 2 + self->startX;
	self->y = player.y + self->startY;

	self->thinkTime++;

	if (self->face != player.face)
	{
		self->face = player.face;

		if (self->thinkTime <= 15)
		{
			self->mental--;
		}

		self->thinkTime = 0;
	}

	if (self->thinkTime >= 60)
	{
		temp = self;

		self = &player;

		self->takeDamage(temp, 1);

		self = temp;

		self->thinkTime = 0;

		if (player.health <= 0)
		{
			self->die();
		}
	}

	if (self->mental <= 0)
	{
		self->x = player.x + player.w / 2 - self->w / 2;

		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		setCustomAction(&player, &slowDown, 3, -1);

		self->action = &fallOff;

		self->thinkTime = 300;

		self->damage = 0;

		self->touch = &entityTouch;

		player.flags &= ~GRABBED;

		self->flags &= ~GRABBING;
	}
}

static void fallOff()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &partAttack;

		self->touch = &partGrab;
	}
}

static void partWait()
{
	int startX, endX;
	int endY;

	startX = getMapStartX();

	endY = getMapStartY();

	endX = startX + SCREEN_WIDTH;

	checkToMap(self);

	if (self->x < startX)
	{
		self->x = startX;
	}

	else if (self->x > endX)
	{
		self->x = endX - self->w - 5;
	}

	if (self->y > endY + SCREEN_HEIGHT)
	{
		printf("Out of bounds\n");

		exit(0);
	}

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}

	if (self->head->maxThinkTime == 0 && self->head->startX != 0)
	{
		self->thinkTime = 60 + prand() % 180;

		self->action = &reform;
	}
}

static void explodeWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->action = &headWait;

		self->touch = &stunnedTouch;

		self->activate = &activate;

		self->takeDamage = &takeDamage;
	}
}

static void headWait()
{
	Target *t;

	checkToMap(self);

	if (self->maxThinkTime <= 0)
	{
		t = getTarget();

		self->x = t->x - self->w / 2;

		self->maxThinkTime = 0;

		self->action = &headReform;
	}
}

static void headReform()
{
	checkToMap(self);

	if (self->startX <= 0)
	{
		if (self->health > 2000)
		{
			self->mental = 15;
		}
		
		else if (self->health > 1000)
		{
			self->mental = 20;
		}
		
		else
		{
			printf("Will now do final set of attacks\n");
		}
	
		self->action = &attackFinished;
	}
}

static void floatInContainer()
{
	if (self->active == TRUE)
	{
		self->health--;
	}

	if (self->health > 0)
	{
		self->thinkTime++;

		if (self->thinkTime >= 360)
		{
			self->thinkTime = 0;
		}

		self->y = self->startY + cos(DEG_TO_RAD(self->thinkTime)) * 32;
	}

	else
	{
		self->flags |= DO_NOT_PERSIST;

		if (self->y < self->endY)
		{
			self->flags &= ~FLY;

			checkToMap(self);
		}

		else if (self->thinkTime > 0)
		{
			self->y = self->endY;

			self->flags |= FLY;

			setEntityAnimation(self, WALK);

			self->animationCallback = &leaveFinish;
		}
	}
}

static void leaveFinish()
{
	self->inUse = FALSE;
}

static void shudder()
{
	self->thinkTime--;

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;
		
		if (self->health > 2000)
		{
			self->mental = 20;
		}
		
		else if (self->health > 1000)
		{
			self->mental = 30;
		}
		
		else
		{
			printf("Will now do final set of attacks\n");
		}

		self->action = &attackFinished;
	}
}

static Target *getTarget()
{
	Target *t = getTargetByName("BLOB_TARGET");

	if (t == NULL)
	{
		printf("Blob Boss could not find target\n");

		exit(1);
	}

	return t;
}
