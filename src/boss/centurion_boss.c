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

#include "../headers.h"

#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../game.h"
#include "../map.h"
#include "../hud.h"
#include "../world/target.h"
#include "../world/explosion.h"
#include "../player.h"
#include "../world/explosion.h"
#include "../geometry.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../system/error.h"

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
static void addHead(void);
static void headWait(void);
static void headFallOff(void);
static void retrieveHead(void);
static void headTakeDamage(Entity *, int);
static void headReturnToBody(void);
static void bodyWait(void);
static void miniCenturionAttackInit(void);
static Entity *addMiniCenturion(void);
static void miniWalk(void);
static void miniCenturionAttack(void);
static void miniAttackWait(void);
static void miniAttackFinish(void);
static void fallout(void);
static void lavaDie(void);

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

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

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
	setEntityAnimation(self, WALK);
	
	if (self->currentFrame == 2 || self->currentFrame == 5)
	{
		if (self->maxThinkTime == 0)
		{
			playSoundToMap("sound/enemy/centurion/walk.ogg", -1, self->x, self->y, 0);

			self->maxThinkTime = 1;
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}
	
	checkToMap(self);

	if (self->currentFrame == 2 || self->currentFrame == 5)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}
	
	if (fabs(self->x - self->targetX) <= fabs(self->speed))
	{
		self->dirX = 0;
		
		setEntityAnimation(self, STAND);
		
		self->mental = 1;
		
		self->action = &entityWait;
		
		self->startX = 60 * 60;
		
		self->startY = 0;
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
		
		setEntityAnimation(self, WALK);
	}
}

static void lookForPlayer()
{
	int r, minX, maxX;
	
	minX = getCameraMinX();
	
	maxX = getCameraMaxX();
	
	if (self->maxThinkTime == 1)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}
	
	if (self->currentFrame == 2 || self->currentFrame == 5)
	{
		if (self->maxThinkTime == 0)
		{
			if (self->flags & ON_GROUND)
			{
				playSoundToMap("sound/enemy/centurion/walk.ogg", -1, self->x, self->y, 0);
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
	
	if (self->thinkTime == 0 && player.health > 0 && prand() % 10 == 0
		&& collision(self->x + (self->face == RIGHT ? self->w : -240), self->y, 240, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		r = prand() % 2;
		
		switch (r)
		{
			case 0:
				self->action = &explosionAttackInit;
				
				self->dirX = 0;
			break;
			
			default:
				self->action = &stompAttackInit;
				
				self->dirX = 0;
			break;
		}
	}
	
	if (self->startY == 0)
	{
		self->startX--;
		
		if (self->startX <= 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("An arrow to the head might work..."));
			
			self->startX = 60 * 60;
		}
	}
}

static void stompAttackInit()
{
	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &stompAttack;

	self->action = &attacking;

	checkToMap(self);
}

static void stompAttack()
{
	setEntityAnimation(self, ATTACK_2);

	playSoundToMap("sound/common/crash.ogg", -1, self->x, self->y, 0);

	shakeScreen(MEDIUM, 60);

	self->thinkTime = 60;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->action = &stompAttackFinish;
}

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, WALK);

		self->action = &lookForPlayer;

		self->thinkTime = 60;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}
}

static void explosionAttackInit()
{
	setEntityAnimation(self, ATTACK_1);

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
	
	setEntityAnimation(e, STAND);

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h - e->h;
	
	e->face = self->face;
	
	e->dirX = e->face == LEFT ? -6 : 6;
	
	e->damage = 1;

	e->action = &explosionMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &explosionTouch;

	e->type = ENEMY;
	
	e->flags |= NO_DRAW|DO_NOT_PERSIST;
	
	e->startX = playSoundToMap("sound/boss/ant_lion/earthquake.ogg", -1, self->x, self->y, -1);
	
	setEntityAnimation(self, ATTACK_2);

	self->thinkTime = 60;

	self->action = &explosionAttackFinish;
	
	checkToMap(self);
}

static void explosionAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, WALK);

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
		self->touch = NULL;
		
		self->dirX = 0;
		
		self->mental = 5;
		
		self->action = &explode;
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

	for (i=0;i<9;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);
		
		e->action = &pieceWait;
		
		e->fallout = &pieceFallout;

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimation(e, i);
		
		e->head = self;
	}
	
	self->damage = 0;
	
	self->health = 0;
	
	self->dirX = 0;

	self->mental = 1;
	
	self->flags &= ~FLY;

	self->flags |= NO_DRAW;
	
	self->takeDamage = NULL;

	self->action = &dieWait;
	
	self->thinkTime = 120;
}

static void dieWait()
{
	Target *t;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		t = getTargetByName(prand() % 2 == 0 ? "CENTURION_TARGET_LEFT" : "CENTURION_TARGET_RIGHT");
		
		if (t == NULL)
		{
			showErrorAndExit("Centurion Boss cannot find target");
		}
		
		self->x = t->x;
		self->y = t->y;
		
		self->mental = 0;
		
		self->thinkTime = 9;
		
		self->action = &reform;
	}
}

static void reform()
{
	if (self->health == -1)
	{
		dropRandomItem(self->x + self->w / 2, self->y);
		
		self->action = &entityDieVanish;
	}
	
	if (self->thinkTime == 0)
	{
		self->thinkTime = 30;
		
		self->action = &reformFinish;
	}
}

static void reformFinish()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->health = self->maxHealth;
		
		self->flags &= ~NO_DRAW;
		
		self->action = &miniCenturionAttackInit;
		
		facePlayer();
		
		self->damage = 1;
		
		self->thinkTime = 20 * 60;
		
		self->takeDamage = &takeDamage;
		
		setEntityAnimation(self, STAND);
	}
}

static void pieceWait()
{
	if (self->head->health == -1)
	{
		self->action = &entityDieNoDrop;
	}
	
	else if (self->head->mental == 0)
	{
		self->action = &pieceReform;
		
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

		self->dirX *= 12;
		self->dirY *= 12;
		
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
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
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

static void addHead()
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
	e->takeDamage = &headTakeDamage;

	e->type = ENEMY;
	
	e->head = self;

	setEntityAnimation(e, STAND);
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
	
	else
	{
		self->flags &= ~FLASH;
	}
	
	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}
	
	else
	{
		self->flags &= ~NO_DRAW;
	}
	
	self->y = self->head->y + self->offsetY;
}

static void headTakeDamage(Entity *other, int damage)
{
	Entity *e;
	
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (strcmpignorecase(other->name, "weapon/normal_arrow") == 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

			enemyPain();
		}

		else
		{
			self->dirX = prand() % 2 == 0 ? -5 : 5;
			
			self->dirY = ITEM_JUMP_HEIGHT;
			
			self->action = &headFallOff;
			
			self->takeDamage = NULL;
			
			self->flags &= ~FLY;
			
			self->head->action = &retrieveHead;
			
			self->head->dirX = 0;
			
			self->head->damage = 0;
			
			self->target = self->head;
			
			self->head->target = self;
			
			self->head->takeDamage = NULL;
			
			self->head->thinkTime = 120;
			
			setEntityAnimation(self->head, STAND);
			
			self->head->animationCallback = NULL;
			
			self->mental = 0;
			
			/* Drop the cell */
			
			if (getInventoryItem("Power Cell") == NULL && getEntityByName("item/power_cell") == NULL)
			{
				e = addPermanentItem("item/power_cell", self->x, self->y);
				
				e->x += self->w / 2 - e->w / 2;
				
				e->dirX = self->dirX < 0 ? 5 : -5;
				
				e->dirY = ITEM_JUMP_HEIGHT;
			}
			
			self->head->startY = 1;
		}
	}
}

static void headFallOff()
{
	int i, minX, maxX;
	
	minX = getCameraMinX();
	
	maxX = getCameraMaxX();
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
		
		if (collision(self->x, self->y, self->w, self->h, self->head->x, self->head->y, self->head->w, self->head->h) == 1)
		{
			self->thinkTime = 60;
			
			self->action = &headReturnToBody;
			
			self->head->action = &bodyWait;
			
			setEntityAnimation(self->head, STAND);
			
			self->face = self->head->face;
			
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
			
			self->dirX *= 3;
			self->dirY *= 3;
			
			self->head->dirX = 0;
			
			self->head->mental = 0;
			
			self->flags |= FLY;
		}
		
		if (self->mental == 0)
		{
			self->mental = 1;
			
			playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);
			
			for (i=0;i<5;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}
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
}


static void retrieveHead()
{
	int minX, maxX;
	
	minX = getCameraMinX();
	
	maxX = getCameraMaxX();
	
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
		
		checkToMap(self);
		
		if (self->thinkTime <= 0)
		{
			self->face = self->target->x < self->x ? LEFT : RIGHT;
			
			setEntityAnimation(self, WALK);
			
			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
		}
	}
	
	else
	{	
		if (self->maxThinkTime == 1)
		{
			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
		}
		
		if (self->currentFrame == 2 || self->currentFrame == 5)
		{
			if (self->maxThinkTime == 0)
			{
				if (self->flags & ON_GROUND)
				{
					playSoundToMap("sound/enemy/centurion/walk.ogg", -1, self->x, self->y, 0);
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
	}
}

static void bodyWait()
{
	checkToMap(self);
	
	if (self->mental == 1)
	{
		self->damage = 1;
		
		self->takeDamage = &takeDamage;
		
		setEntityAnimation(self, WALK);
		
		self->action = &lookForPlayer;
	}
}

static void headReturnToBody()
{
	self->thinkTime--;
	
	checkToMap(self);
	
	if (atTarget() || self->thinkTime <= 0)
	{
		self->head->mental = 1;
		
		self->action = &headWait;
		self->takeDamage = &headTakeDamage;
		
		self->health = self->maxHealth;
	}
}

static void miniCenturionAttackInit()
{
	setEntityAnimation(self, STAND);
	
	self->thinkTime = 20;
	
	self->mental = 3 + prand() % 3;
	
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
		
		e->face = self->face;
		
		e->dirX = (e->face == LEFT ? -e->speed : e->speed);
		
		e->dirY = -8;
		
		e->head = self;
		
		self->mental--;
		
		if (self->mental <= 0)
		{
			self->action = &miniAttackWait;
			
			self->thinkTime = 60 * 20;
			
			self->endX = 600;
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
		
		e->mental = -1;
	}
	
	else
	{
		loadProperties("enemy/mini_centurion", e);
	}
	
	e->x = self->x;
	e->y = self->y;

	e->type = ENEMY;
	
	e->flags |= LIMIT_TO_SCREEN;

	setEntityAnimation(e, STAND);
	
	e->action = &miniWalk;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &entityDie;
	e->fallout = &entityDie;
	e->reactToBlock = &changeDirection;
	e->pain = &enemyPain;
	
	return e;
}

static void miniWalk()
{
	Entity *e;
	
	if (self->flags & ON_GROUND)
	{
		moveLeftToRight();
		
		/* Prevent the Boss from attack while there are minis on the screen */
		
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

static void miniAttackWait()
{
	self->thinkTime--;
	
	self->endX--;
	
	if (self->endX <= 0 || self->thinkTime <= 0)
	{
		setEntityAnimation(self, WALK);
		
		if (self->currentFrame == 2 || self->currentFrame == 5)
		{
			if (self->maxThinkTime == 0)
			{
				playSoundToMap("sound/enemy/centurion/walk.ogg", -1, self->x, self->y, 0);

				self->maxThinkTime = 1;
			}

			self->dirX = 0;
		}

		else
		{
			self->maxThinkTime = 0;
		}
		
		checkToMap(self);

		if (self->currentFrame == 2 || self->currentFrame == 5)
		{
			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
		}
	}
	
	else
	{
		checkToMap(self);
	}
	
	if (!(self->flags & ON_GROUND))
	{
		self->thinkTime = 30;
		
		self->dirX = 0;
		
		setEntityAnimation(self, STAND);
		
		self->action = &miniAttackFinish;
	}
}

static void miniAttackFinish()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, WALK);
			
			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
			
			self->action = &lookForPlayer;
		}
	}
	
	checkToMap(self);
}

static void fallout()
{
	setEntityAnimation(self, STAND);
	
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
	}
}
