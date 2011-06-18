/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../graphics/graphics.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../map.h"
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../item/item.h"
#include "../game.h"
#include "../audio/music.h"
#include "../audio/audio.h"
#include "../graphics/gib.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../geometry.h"
#include "../world/target.h"
#include "../hud.h"
#include "../player.h"
#include "../graphics/decoration.h"
#include "../system/error.h"
#include "../enemy/thunder_cloud.h"

extern Entity *self, player;

static void init(void);
static void entityWait(void);
static void attackFinished(void);
static void entityWait(void);
static void statueAttackInit(void);
static void statueAttack(void);
static void statueRise(void);
static void statueRiseWait(void);
static void holdPersonInit(void);
static void holdPerson(void);
static void holdPersonSpellMove(void);
static void holdPersonWait(void);
static void createShieldInit(void);
static void createShield(void);
static void shieldWait(void);
static void shieldTakeDamage(Entity *, int);
static void shieldDie(void);
static void pieceWait(void);
static void holdPersonBackWait(void);
static void holdPersonPieceMove(void);
static void castLightningBolt(void);
static void lightningBolt(void);
static void floorRiseInit(void);
static void floorRiseWait(void);
static void floorRise(void);
static void floorTouch(Entity *);
static void lightningDropInit(void);
static void lightningDrop(void);
static void lightningGroundAttack(void);
static void groundLightningMove(void);

Entity *addSorceror2(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		setContinuePoint(FALSE, self->name, NULL);
		
		self->targetX = self->x;
		self->targetY = self->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		self->thinkTime = 120;
		
		self->action = &createShieldInit;
	}
	
	checkToMap(self);
}

static void attackFinished()
{
	self->thinkTime = 180;
	
	self->action = &entityWait;
}

static void entityWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		if (getEntityByName("enemy/flame_statue") != NULL)
		{
			switch (prand() % 2)
			{
				case 1:
					self->action = &floorRiseInit;
				break;
				
				default:
					self->action = &holdPersonInit;
				break;
			}
		}
		
		else
		{
			switch (prand() % 2)
			{
				case 0:
					self->action = &holdPersonInit;
				break;
				
				case 1:
					self->action = &floorRiseInit;
				break;
				
				default:
					self->action = &statueAttackInit;
				break;
			}
		}
	}
	
	self->action = &lightningDropInit;
}

static void lightningDropInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	self->layer = BACKGROUND_LAYER;

	setEntityAnimation(self, "RAISE_ARMS");

	playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
	
	t = getTargetByName("SORCEROR_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	self->y = t->y;

	self->thinkTime = 60;

	self->flags |= FLY;

	self->action = &lightningDrop;
}

static void lightningDrop()
{
	int i;

	if (self->thinkTime > 0)
	{
		self->flags |= INVULNERABLE;

		self->thinkTime--;

		self->x = player.x + player.w / 2 - self->w / 2;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags &= ~(FLY|NO_DRAW);

			self->dirY = 0;
			
			setEntityAnimation(self, "STAND");
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/enemy/red_grub/thud.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(LIGHT, 15);

		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}
		
		self->mental = 5;

		self->thinkTime = 30;

		self->action = &lightningGroundAttack;
	}
}

static void lightningGroundAttack()
{
	int i;
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		for (i=0;i<2;i++)
		{
			e = getFreeEntity();
			
			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Sorceror's Ground Lightning");
			}
			
			loadProperties("boss/sorceror_ground_lightning", e);
			
			setEntityAnimation(e, "FRONT");
			
			e->face = RIGHT;
			
			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y + self->h - e->h;
			
			e->dirX = i == 0 ? 2 : -2;
			
			e->action = &groundLightningMove;
			
			e->draw = &drawLoopingAnimationToMap;
			
			e->touch = &entityTouch;
		}
		
		self->mental--;
		
		self->thinkTime = 30;
	}
	
	checkToMap(self);
}

static void groundLightningMove()
{
	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
	
	checkToMap(self);
}

static void statueAttackInit()
{
	setEntityAnimation(self, "RAISE_ARMS");
	
	self->thinkTime = 15;
	
	self->action = &statueAttack;
}

static void statueAttack()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = addEnemy("enemy/flame_statue", 0, 0);
		
		e->health = 5;
		
		e->layer = BACKGROUND_LAYER;
		
		e->x = player.x + player.w / 2 - e->w / 2;
		
		e->x -= player.w * 2;
		
		e->mental = 0;
		
		e->y = getMapFloor(self->x, self->y);
		
		e->targetY = e->y - e->h;
		
		e->action = &statueRise;
		
		e->thinkTime = 240;
		
		e = addEnemy("enemy/flame_statue", 0, 0);
		
		e->health = 5;
		
		e->layer = BACKGROUND_LAYER;
		
		e->x = player.x + player.w / 2 - e->w / 2;
		
		e->x += player.w * 2;
		
		e->mental = 1;
		
		e->y = getMapFloor(self->x, self->y);
		
		e->targetY = e->y - e->h;
		
		e->action = &statueRise;
		
		e->thinkTime = 240;
		
		self->thinkTime = 600;
		
		self->action = &statueRiseWait;
	}
}

static void statueRise()
{
	Entity *e;

	if (self->y > self->targetY)
	{
		self->y -= 12;

		if (self->y <= self->targetY)
		{
			self->y = self->targetY;

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
	}

	else
	{
		self->thinkTime--;
		
		self->action = self->resumeNormalFunction;
	}
}

static void statueRiseWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void holdPersonInit()
{
	setEntityAnimation(self, "RAISE_ARMS");
	
	self->thinkTime = 15;
	
	self->action = &holdPerson;
	
	self->mental = 3;
}

static void holdPerson()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Sorceror's Hold Person");
		}
		
		loadProperties("boss/sorceror_hold_person_spell", e);
		
		setEntityAnimation(e, "FRONT");
		
		e->face = RIGHT;
		
		e->x = player.x + player.w / 2 - e->w / 2;
		e->y = getMapStartY() - e->h;
		
		e->layer = FOREGROUND_LAYER;
		
		e->mental = self->mental;
		
		e->action = &holdPersonSpellMove;
		
		e->draw = &drawLoopingAnimationToMap;
		
		self->mental--;
		
		if (self->mental <= 0)
		{
			self->action = &holdPersonWait;
			
			self->thinkTime = 300;
		}
		
		else
		{
			self->thinkTime = 5;
		}
	}
}

static void holdPersonSpellMove()
{
	Entity *e;
	
	if (self->target == NULL)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Sorceror's Hold Person");
		}
		
		loadProperties("boss/sorceror_hold_person_spell", e);
		
		setEntityAnimation(e, "BACK");
		
		e->face = RIGHT;
		
		e->action = &holdPersonBackWait;
		e->draw = &drawLoopingAnimationToMap;
		
		e->x = self->x;
		e->y = self->y;
		
		e->layer = BACKGROUND_LAYER;
		
		self->target = e;
		
		e->head = self;
	}
	
	switch (self->mental)
	{
		case 3:
			self->targetY = player.y + player.h / 2 - self->h / 2 - 16;
		break;
		
		case 2:
			self->targetY = player.y + player.h / 2 - self->h / 2;
		break;
		
		default:
			self->targetY = player.y + player.h / 2 - self->h / 2 + 16;
		break;
	}
	
	if (self->y < self->targetY)
	{
		self->y += 12;
		
		if (self->y >= self->targetY)
		{
			self->y = self->targetY;
			
			self->face = player.face;
		}
	}
	
	else
	{
		if (self->face != player.face)
		{
			self->face = player.face;
			
			self->health--;
			
			if (self->health <= 0)
			{
				setEntityAnimation(self, "LEFT_PIECE");
				
				self->layer = FOREGROUND_LAYER;
				
				self->dirX = -4;
				
				self->action = &holdPersonPieceMove;
				
				self->thinkTime = 30;
			}
		}
	}
	
	player.x = self->x + self->w / 2 - player.w / 2;
}

static void holdPersonBackWait()
{
	self->x = self->head->x;
	self->y = self->head->y;
	
	if (self->head->health <= 0)
	{
		setEntityAnimation(self, "RIGHT_PIECE");
		
		self->layer = FOREGROUND_LAYER;
		
		self->dirX = 4;
		
		self->action = &holdPersonPieceMove;
		
		self->thinkTime = 30;
	}
}

static void holdPersonPieceMove()
{
	self->x += self->dirX;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void holdPersonWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->mental = 3;
		
		self->action = &castLightningBolt;
	}
}

static void castLightningBolt()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add lightning");
		}

		loadProperties("enemy/lightning", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapCeiling(self->x, self->y);

		e->startY = e->targetY;
		e->endY   = getMapFloor(e->targetX, e->targetY);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		setEntityAnimation(e, "STAND");

		e->action = &lightningBolt;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= FLY|DO_NOT_PERSIST;

		setEntityAnimation(e, "STAND");

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 0;

			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void lightningBolt()
{
	int i, middle;
	Entity *e;

	self->flags |= NO_DRAW;

	self->thinkTime--;

	middle = -1;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/enemy/thunder_cloud/lightning.ogg", -1, self->targetX, self->startY, 0);

		for (i=self->startY;i<self->endY;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add lightning");
			}

			loadProperties("enemy/lightning", e);

			setEntityAnimation(e, "STAND");

			if (i == self->startY)
			{
				middle = self->targetX + self->w / 2 - e->w / 2;
			}

			e->x = middle;
			e->y = i;

			e->action = &lightningWait;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->currentFrame = prand() % 6;

			e->face = RIGHT;

			e->thinkTime = 15;
		}

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}
}

static void createShieldInit()
{
	setEntityAnimation(self, "RAISE_ARMS");
	
	self->thinkTime = 30;
	
	self->action = &createShield;
	
	checkToMap(self);
}

static void createShield()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Sorceror's Shield");
		}

		loadProperties("boss/sorceror_shield", e);

		e->x = self->x;
		e->y = self->y;

		e->action = &shieldWait;

		e->draw = &drawLoopingAnimationToMap;
		
		e->touch = &entityTouch;
		
		e->takeDamage = &shieldTakeDamage;
		
		e->pain = &enemyPain;
		
		e->die = &shieldDie;

		e->type = ENEMY;

		e->head = self;

		e->alpha = 128;
		
		e->face = RIGHT;

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;
		
		self->startX = 1;

		self->action = &attackFinished;
	}
	
	checkToMap(self);
}

static void shieldWait()
{
	float radians;

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	self->thinkTime += 5;

	radians = DEG_TO_RAD(self->thinkTime);

	self->alpha = 128 + (64 * cos(radians));
}

static void shieldTakeDamage(Entity *other, int damage)
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

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void shieldDie()
{
	int i, x, y;
	Entity *e;
	
	x = self->x;
	y = self->y;
	
	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Sorceror's Shield Piece");
		}

		loadProperties("boss/sorceror_shield_piece", e);

		e->x = x;
		e->y = y;

		e->action = &pieceWait;

		e->draw = &drawLoopingAnimationToMap;

		e->type = ENEMY;

		e->alpha = 128;
		
		e->face = RIGHT;

		setEntityAnimationByID(e, i);
		
		if (x == self->x)
		{
			x += e->w;
		}
		
		else
		{
			x = self->x;
			
			y += e->h;
		}
		
		switch (i)
		{
			case 0:
				e->dirX = -3;
				e->dirY = -1.5f;
			break;
			
			case 1:
				e->dirX = 3;
				e->dirY = -1.5f;
			break;
			
			case 2:
				e->dirX = -3;
			break;
			
			case 3:
				e->dirX = 3;
			break;
			
			case 4:
				e->dirX = -3;
				e->dirY = 1.5f;
			break;
			
			default:
				e->dirX = 3;
				e->dirY = 1.5f;
			break;
		}
	}
	
	self->head->startX = 0;
	
	self->inUse = FALSE;
}

static void pieceWait()
{
	self->alpha -= 6;
	
	if (self->alpha <= 0)
	{
		self->inUse = FALSE;
	}
	
	self->x += self->dirX;
	self->y += self->dirY;
}

static void floorRiseInit()
{
	EntityList *list = getEntitiesByObjectiveName("SORCEROR_FLOOR");
	EntityList *l;
	Entity *e;
	int playerMid = player.x + player.w / 2;
	
	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;
		
		if (playerMid >= e->x && playerMid <= e->x + e->w)
		{
			e->head = self;
			
			e->action = &floorRise;
			
			e->touch = &floorTouch;
			
			self->mental = 1;
			
			break;
		}
	}
	
	self->action = self->mental == 1 ? &floorRiseWait : &entityWait;
	
	freeEntityList(list);
}

static void floorRiseWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}
}

static void floorRise()
{
	self->dirY = -3.5;
	
	checkToMap(self);
	
	if (self->dirY == 0)
	{
		self->head->mental = 0;
		
		self->die();
	}
}

static void floorTouch(Entity *other)
{
	int bottomBefore;
	float dirX;
	Entity *temp;

	if (other->type == PLAYER && self->damage != 0)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}

	if (other->type == PLAYER && other->dirY > 0 && !(other->flags & NO_DRAW))
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				if (self->dirY < 0)
				{
					other->y -= self->dirY;

					other->dirY = self->dirY;

					dirX = other->dirX;

					other->dirX = 0;

					checkToMap(other);

					other->dirX = dirX;

					if (other->dirY == 0)
					{
						/* Gib the player */

						temp = self;

						self = other;

						freeEntityList(playerGib());

						self = temp;
					}
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}
		}
	}
}
