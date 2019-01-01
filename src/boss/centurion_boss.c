/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../audio/audio.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../item/item.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/explosion.h"
#include "../world/target.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void initialise(void);
static void doIntro(void);
static void entityWait(void);
static void stompAttackInit(void);
static void stompAttack(void);
static void stompAttackFinish(void);
static void attacking(void);
static void die(void);
static void reform(void);
static void dieWait(void);
static void pieceWait(void);
static void pieceReform(void);
static void pieceFallout(void);
static void reformFinish(void);
static void explosionAttackInit(void);
static void explosionAttack(void);
static void explosionAttackFinish(void);
static void explosionMove(void);
static void explode(void);
static void explosionTouch(Entity *);
static void takeDamage(Entity *, int);
static Entity *addHead(void);
static void headWait(void);
static void headTakeDamage(Entity *, int);
static void miniCenturionAttackInit(void);
static Entity *addMiniCenturion(void);
static void miniWalk(void);
static void miniCenturionAttack(void);
static void fallout(void);
static void lavaDie(void);
static void moveToBody(void);
static void followPlayer(void);
static void dropOnPlayer(void);
static void dropWait(void);
static void creditsMove(void);
static void creditsMiniMove(void);
static void creditsMoveRegular(void);
static void creditsReformFinish(void);

Entity *addCenturionBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Centurion Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->fallout = &fallout;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initialise()
{
	Target *t;

	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		addHead();

		self->thinkTime = 60;

		self->action = &doIntro;

		t = getTargetByName("CENTURION_INTRO_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Centurion Boss cannot find target");
		}

		self->targetX = t->x;

		self->face = self->targetX < self->x ? LEFT : RIGHT;
	}

	checkToMap(self);
}

static void doIntro()
{
	setEntityAnimation(self, "WALK");

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);

			self->maxThinkTime = 1;
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	checkToMap(self);

	if (self->offsetX != 0)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (fabs(self->x - self->targetX) <= fabs(self->speed))
	{
		self->dirX = 0;

		setEntityAnimation(self, "STAND");

		self->mental = 1;

		self->action = &entityWait;

		self->startX = 60 * 10;

		self->startY = 0;

		self->endY = 0;

		self->endX = 0;
	}
}

static void entityWait()
{
	checkToMap(self);

	if (self->mental == 2)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		playDefaultBossMusic();

		self->action = &lookForPlayer;

		self->takeDamage = &takeDamage;

		self->die = &die;

		setEntityAnimation(self, "WALK");
	}
}

static void lookForPlayer()
{
	int minX, maxX;

	minX = getCameraMinX();

	maxX = getCameraMaxX();

	if (self->maxThinkTime == 1)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			if (self->flags & ON_GROUND)
			{
				playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);
			}

			self->maxThinkTime = 1;
		}

		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	if (self->maxThinkTime == 0 && (self->dirX == 0 || isAtEdge(self) == TRUE))
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	checkToMap(self);

	if (self->x < minX)
	{
		self->dirX = 0;

		self->x = minX;
	}

	else if (self->x + self->w > maxX)
	{
		self->dirX = 0;

		self->x = maxX - self->w;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	self->endX--;

	if (self->endX <= 0)
	{
		self->endX = 0;
	}

	if (self->thinkTime == 0 && player.health > 0 && prand() % 10 == 0 && self->endX == 0
		&& collision(self->x + (self->face == RIGHT ? self->w : -240), self->y, 240, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		self->action = &explosionAttackInit;

		self->dirX = 0;
	}
}

static void stompAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &stompAttack;

	self->action = &attacking;

	checkToMap(self);
}

static void stompAttack()
{
	setEntityAnimation(self, "ATTACK_2");

	playSoundToMap("sound/common/crash", -1, self->x, self->y, 0);

	shakeScreen(MEDIUM, 60);

	self->thinkTime = 60;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->endY++;

	activateEntitiesWithRequiredName(self->objectiveName, TRUE);

	if (self->endY == 3)
	{
		setInfoBoxMessage(180, 255, 255, 255, _("The glass looks very weak now..."));
	}

	self->action = &stompAttackFinish;
}

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->action = &miniCenturionAttackInit;

		self->thinkTime = 60;
	}
}

static void explosionAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &explosionAttack;

	self->action = &attacking;

	checkToMap(self);
}

static void explosionAttack()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a moving explosion");
	}

	loadProperties("common/explosion", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h - e->h;

	e->face = self->face;

	e->dirX = e->face == LEFT ? -6 : 6;

	e->damage = 1;

	e->mental = prand() % 2;

	e->action = &explosionMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &explosionTouch;

	e->type = ENEMY;

	e->flags |= NO_DRAW|DO_NOT_PERSIST;

	if (e->mental != 0)
	{
		e->flags |= LIMIT_TO_SCREEN;
	}

	e->thinkTime = 300;

	e->startX = playSoundToMap("sound/boss/ant_lion/earthquake", -1, self->x, self->y, -1);

	setEntityAnimation(self, "ATTACK_2");

	self->thinkTime = 60;

	self->action = &explosionAttackFinish;

	checkToMap(self);
}

static void explosionAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "WALK");

		self->action = &lookForPlayer;

		self->thinkTime = 60;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	checkToMap(self);
}

static void explosionMove()
{
	Entity *e;

	e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

	if (e != NULL)
	{
		e->y -= prand() % e->h;
	}

	checkToMap(self);

	if (isAtEdge(self) == TRUE || self->dirX == 0)
	{
		if (self->mental <= 0)
		{
			self->touch = NULL;

			self->dirX = 0;

			self->mental = 5;

			self->action = &explode;
		}

		else
		{
			self->mental--;

			if (self->mental == 0)
			{
				self->flags &= ~LIMIT_TO_SCREEN;
			}

			self->face = self->face == LEFT ? RIGHT : LEFT;

			self->dirX = (self->face == LEFT ? -6 : 6);
		}
	}
}

static void explosionTouch(Entity *other)
{
	entityTouch(other);

	if (other->type == PLAYER)
	{
		self->touch = NULL;

		self->dirX = 0;

		self->mental = 5;

		self->action = &explode;

		self->thinkTime = 0;
	}
}

static void explode()
{
	int x, y;
	Entity *e;

	self->thinkTime--;

	stopSound(self->startX);

	self->startX = -1;

	if (self->thinkTime <= 0)
	{
		x = self->x + self->w / 2;
		y = self->y + self->h / 2;

		x += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);

		e = addExplosion(x, y);

		e->type = ENEMY;

		e->damage = 1;

		self->mental--;

		self->thinkTime = 10;

		if (self->mental == 0)
		{
			self->inUse = FALSE;
		}
	}
}

static void attacking()
{
	checkToMap(self);
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	snprintf(name, sizeof(name), "boss/gold_centurion_boss_piece");

	playSoundToMap("sound/enemy/centurion/centurion_die", BOSS_CHANNEL, self->x, self->y, 0);

	for (i=0;i<8;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->action = &pieceWait;

		e->fallout = &pieceFallout;

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->head = self;

		e->creditsAction = &pieceWait;
	}

	self->damage = 0;

	self->health = 0;

	self->dirX = 0;

	self->thinkTime = 240;

	self->mental = 1;

	self->flags &= ~FLY;

	self->flags |= NO_DRAW;

	self->takeDamage = NULL;

	self->action = &dieWait;

	self->creditsAction = &dieWait;
}

static void dieWait()
{
	Entity *e;

	if (self->mental == 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->mental = 3;

			e = addHead();

			e->y = self->y - 32;

			if (self->face == LEFT)
			{
				e->x = getMapStartX() + SCREEN_WIDTH + e->w * 6;

				e->targetX = self->x + self->w - e->w - e->offsetX;
			}

			else
			{
				e->x = getMapStartX() - e->w * 6;

				e->targetX = self->x + e->offsetX;
			}

			e->targetY = e->y;

			e->face = self->face;

			calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

			e->dirX *= 2;
			e->dirY *= 2;

			e->action = &moveToBody;

			e->creditsAction = &moveToBody;

			e->thinkTime = 60;

			self->action = &reform;

			self->creditsAction = &reform;
		}
	}
}

static void reform()
{
	if (self->health == -1)
	{
		self->action = &entityDieVanish;

		fadeBossMusic();
	}
}

static void moveToBody()
{
	if (self->mental == 0 && atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->dirY = 1;

			self->mental = 1;
		}
	}

	else if (self->mental == 1 && self->y >= self->head->y + self->offsetY)
	{
		self->y = self->head->y + self->offsetY;

		self->dirY = 0;

		self->head->mental = 3;

		self->action = &headWait;

		self->creditsAction = &headWait;

		self->mental = 0;

		self->head->health = self->head->maxHealth;

		self->head->action = &reformFinish;

		self->head->creditsAction = &creditsReformFinish;
	}

	checkToMap(self);
}

static void reformFinish()
{
	self->health = self->maxHealth;

	self->flags &= ~NO_DRAW;

	if (self->endY < 3)
	{
		self->action = &stompAttackInit;
	}

	else
	{
		self->action = &lookForPlayer;

		setEntityAnimation(self, "WALK");
	}

	facePlayer();

	self->damage = 1;

	self->takeDamage = &takeDamage;

	setEntityAnimation(self, "STAND");

	checkToMap(self);
}

static void pieceWait()
{
	if (self->head->health == -1)
	{
		self->action = &entityDieNoDrop;
	}

	else if (self->head->mental == 3)
	{
		self->action = &pieceReform;

		self->creditsAction = &pieceReform;

		if (self->face == LEFT)
		{
			self->targetX = self->head->x + self->head->w - self->w - self->offsetX;
		}

		else
		{
			self->targetX = self->head->x + self->offsetX;
		}

		self->targetY = self->head->y + self->offsetY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->flags |= FLY;

		self->touch = NULL;
	}

	if ((self->flags & ON_GROUND) && !(self->flags & FLY))
	{
		self->dirX = 0;
	}

	checkToMap(self);
}

static void pieceReform()
{
	if (atTarget())
	{
		if (self->mental == 0)
		{
			self->mental = 1;

			self->head->thinkTime--;
		}

		else
		{
			if (self->head->health == self->head->maxHealth)
			{
				self->inUse = FALSE;
			}
		}
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void pieceFallout()
{
	self->head->health = -1;

	self->inUse = FALSE;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

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
			self->animationCallback = NULL;

			self->damage = 0;

			self->die();
		}
	}
}

static Entity *addHead()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Centurion Boss Head");
	}

	loadProperties("boss/centurion_boss_head", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &headWait;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->creditsAction = &headWait;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");

	return e;
}

static void headWait()
{
	self->frameTimer = self->head->frameTimer;
	self->currentFrame = self->head->currentFrame;

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else if (!(self->flags & INVULNERABLE))
	{
		self->flags &= ~FLASH;
	}

	if (self->head->health <= 0)
	{
		self->startY = self->y - 64;

		self->damage = 1;

		self->thinkTime = 60;

		self->action = &dropWait;

		self->takeDamage = &headTakeDamage;
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}

	self->y = self->head->y + self->offsetY;
}

static void headTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			enemyPain();
		}

		else
		{
			self->animationCallback = NULL;

			self->damage = 0;

			self->head->mental = 2;

			self->head->thinkTime = 120;

			entityDieNoDrop();
		}
	}
}

static void miniCenturionAttackInit()
{
	setEntityAnimation(self, "STAND");

	self->thinkTime = 20;

	self->mental = 1 + prand() % 3;

	self->action = &miniCenturionAttack;

	self->dirX = 0;
}

static void miniCenturionAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addMiniCenturion();

		e->y = self->y + self->h / 2 - e->h / 2;

		e->face = self->face == LEFT ? RIGHT : LEFT;

		e->dirX = (self->face == RIGHT ? -e->speed : e->speed);

		e->dirY = -8;

		e->head = self;

		self->mental--;

		if (self->mental <= 0)
		{
			setEntityAnimation(self, "WALK");

			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

			self->action = &lookForPlayer;
		}

		else
		{
			self->thinkTime = 60;
		}
	}

	checkToMap(self);
}

static Entity *addMiniCenturion()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mini Centurion");
	}

	if (prand() % 4 == 0)
	{
		loadProperties("enemy/mini_red_centurion", e);

		self->mental = -1;
	}

	else
	{
		loadProperties("enemy/mini_centurion", e);
	}

	e->x = self->x;
	e->y = self->y;

	e->type = ENEMY;

	e->flags |= LIMIT_TO_SCREEN;

	setEntityAnimation(e, "STAND");

	e->action = &miniWalk;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->fallout = &entityDie;
	e->reactToBlock = &changeDirection;
	e->pain = &enemyPain;
	e->die = &entityDie;

	e->creditsAction = &creditsMiniMove;

	return e;
}

static void miniWalk()
{
	Entity *e;

	if (self->flags & ON_GROUND)
	{
		moveLeftToRight();

		/* Prevent the Boss from attacking while there are minis on the screen */

		self->head->endX = 30;

		if (self->mental == -1)
		{
			self->thinkTime--;

			if (self->thinkTime <= 60)
			{
				if (self->thinkTime % 3 == 0)
				{
					self->flags ^= FLASH;
				}

				if (self->thinkTime <= 0)
				{
					e = addExplosion(self->x, self->y);

					e->x = self->x - self->w / 2 + e->w / 2;
					e->y = self->y - self->h / 2 + e->h / 2;

					e->damage = 1;

					self->inUse = FALSE;
				}
			}
		}
	}

	else
	{
		checkToMap(self);
	}
}

static void fallout()
{
	setEntityAnimation(self, "STAND");

	self->element = FIRE;

	self->dirX = 0;

	self->dirY = 0;

	self->flags |= DO_NOT_PERSIST;

	self->action = &lavaDie;
}

static void lavaDie()
{
	self->dirY = 0.5;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->inUse = FALSE;

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		fadeBossMusic();
	}
}

static void followPlayer()
{
	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position above the player */

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->thinkTime = 15;

		self->action = &dropOnPlayer;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}

	checkToMap(self);
}

static void dropOnPlayer()
{
	int i;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->flags &= ~FLY;

		if ((self->flags & ON_GROUND))
		{
			playSoundToMap("sound/enemy/red_grub/thud", BOSS_CHANNEL, self->x, self->y, 0);

			shakeScreen(LIGHT, 15);

			self->thinkTime = 15;

			self->action = &dropWait;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}

		checkToMap(self);
	}
}

static void dropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		if (self->y < self->startY)
		{
			self->y = self->startY;

			self->dirY = 0;

			self->action = &followPlayer;
		}

		else
		{
			self->dirY = -4;
		}
	}

	checkToMap(self);
}

static void creditsMove()
{
	float dirX;

	setEntityAnimation(self, "WALK");

	self->flags &= ~NO_DRAW;

	if (self->maxThinkTime == 1)
	{
		self->dirX = self->speed;
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			self->thinkTime++;

			self->maxThinkTime = 1;

			playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}

	if (self->thinkTime >= 10)
	{
		self->dirX = 0;

		setEntityAnimation(self, "STAND");

		self->die = &die;

		self->die();

		self->mental = 2;
	}
}

static void creditsMoveRegular()
{
	float dirX;
	Entity *e;

	setEntityAnimation(self, "WALK");

	if (self->maxThinkTime == 1)
	{
		self->dirX = self->speed;
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			self->maxThinkTime = 1;

			playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}

	if (prand() % 100 == 0)
	{
		e = addMiniCenturion();

		e->x = self->startX;
		e->y = self->startY;
	}
}

static void creditsReformFinish()
{
	self->health = self->maxHealth;

	self->flags &= ~NO_DRAW;

	self->creditsAction = &creditsMoveRegular;

	setEntityAnimation(self, "WALK");

	self->face = RIGHT;
}

static void creditsMiniMove()
{
	self->face = RIGHT;

	self->flags &= ~LIMIT_TO_SCREEN;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
