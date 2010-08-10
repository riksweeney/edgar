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
#include "../graphics/decoration.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../player.h"
#include "../system/error.h"
#include "../enemy/enemies.h"
#include "../item/item.h"
#include "../item/bomb.h"
#include "../geometry.h"
#include "../world/explosion.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void attackFinished(void);
static void fallout(void);
static void entityWait(void);
static void changeSidesSink(void);
static void changeSidesWait(void);
static void changeSidesRise(void);
static void takeDamage(Entity *, int);
static void addClaws(void);
static void createArm(Entity *);
static void clawWait(void);
static void armPartWait(void);
static void alignArmToClaw(void);
static void hover(void);
static void punchAttackInit(void);
static void punchAttack(void);
static void punchWait(void);
static void punchAttackFinish(void);
static void punchReturnToNormal(void);
static void slimeAttackInit(void);
static void slimeAttackMoveToTarget(void);
static void slimeAttack(void);
static void slimeAttackMouthClose(void);
static void slimeAttack(void);
static void slimeAttackFinish(void);
static void touch(Entity *);
static void punchReactToBlock(void);
static void grabTouch(Entity *);
static void grabAttackInit(void);
static void grabAttack(void);
static void grabWait(void);
static void clawTakeDamage(Entity *, int);
static void dropPlayer(void);

Entity *addSewerBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sewer Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;
	e->touch = NULL;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->flags |= ATTACKING;
		
		setContinuePoint(FALSE, self->name, NULL);
		
		self->endX = 0;
		
		addClaws();
		
		self->action = &doIntro;
		
		self->touch = &touch;
		
		self->takeDamage = &takeDamage;
		
		self->dirY = -self->speed;
	}
}

static void doIntro()
{
	checkToMap(self);
	
	if (self->y <= self->startY)
	{
		self->y = self->startY;
		
		self->dirY = 0;
		
		self->action = &introPause;
	}
}

static void introPause()
{
	playDefaultBossMusic();

	initBossHealthBar();
	
	self->action = &attackFinished;
}

static void attackFinished()
{
	setEntityAnimation(self, STAND);
	
	self->thinkTime = 180;
	
	self->action = &entityWait;
}

static void entityWait()
{
	int action;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->element != WATER)
		{
			action = prand() % 5;
			
			switch (action)
			{
				case 0:
				case 1:
					setEntityAnimation(self, ATTACK_4);
					
					self->mental = -5;
					
					self->action = &punchWait;
				break;
				
				case 2:
				case 3:
					setEntityAnimation(self, ATTACK_4);
					
					self->mental = -10;
					
					self->action = &punchWait;
				break;
				
				default:
					self->dirY = self->speed;
					
					self->action = &changeSidesSink;
				break;
			}
		}
		
		else
		{
			self->action = &slimeAttackInit;
		}
	}
	
	checkToMap(self);
	
	hover();
}

static void punchWait()
{
	checkToMap(self);
	
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}
}

static void changeSidesSink()
{
	checkToMap(self);
	
	if (self->y > self->endY)
	{
		self->y = self->endY;
		
		self->dirY = 0;
		
		self->thinkTime = 30;
		
		self->action = &changeSidesWait;
	}
}

static void changeSidesWait()
{
	int rand;
	Target *t;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		rand = prand() % 2;
		
		t = getTargetByName(rand == 0 ? "SEWER_BOSS_TARGET_RIGHT" : "SEWER_BOSS_TARGET_LEFT");
		
		if (t == NULL)
		{
			showErrorAndExit("Sewer Boss cannot find target");
		}
		
		self->x = t->x;
		
		self->face = rand == 0 ? LEFT : RIGHT;
		
		self->action = &changeSidesRise;
		
		self->dirY = -self->speed;
	}
}

static void changeSidesRise()
{
	self->dirY = -self->speed;
	
	if (self->y <= self->startY)
	{
		self->y = self->startY;
		
		self->dirY = 0;
		
		self->action = &attackFinished;
	}
	
	checkToMap(self);
}

static void fallout()
{
	/* Do nothing since the boss cannot fall out of the map */
}

static void takeDamage(Entity *other, int damage)
{
	playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

	if (prand() % 10 == 0)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
	}

	setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
}

static void addClaws()
{
	int i;
	Entity *e;
	
	for (i=0;i<2;i++)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Sewer Boss Arm");
		}
		
		loadProperties("boss/sewer_boss_claw", e);
		
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->die = &entityDieNoDrop;
		e->action = &clawWait;
		e->takeDamage = &clawTakeDamage;
		
		e->head = self;
		
		e->x = self->x + self->w / 2 - e->w / 2;
		
		e->startX = e->x;
		
		e->layer = (i == 0 ? BACKGROUND_LAYER : FOREGROUND_LAYER);
		
		createArm(e);
	}
	
	self->endX = 2;
}

static void createArm(Entity *top)
{
	int i;
	Entity **body, *head;

	top->x = top->endX;
	top->y = top->endY;

	body = (Entity **)malloc(top->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Sewer Boss Arm...\n", top->mental * (int)sizeof(Entity *));
	}

	/* Create in reverse order so that it is drawn correctly */

	resetEntityIndex();

	for (i=top->mental-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			showErrorAndExit("No free slots to add a Sewer Boss Arm part");
		}

		loadProperties("boss/sewer_boss_arm", body[i]);

		body[i]->x = top->x;
		body[i]->y = top->y;

		body[i]->action = &armPartWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;

		body[i]->type = ENEMY;
		
		body[i]->layer = top->layer;

		setEntityAnimation(body[i], STAND);
	}

	/* Recreate the claw so that it's on top */

	head = getFreeEntity();

	if (head == NULL)
	{
		showErrorAndExit("No free slots to add a Sewer Boss Claw");
	}

	*head = *top;

	top->inUse = FALSE;

	top = head;

	/* Link the sections */

	for (i=top->mental-1;i>=0;i--)
	{
		if (i == 0)
		{
			top->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];
		}

		body[i]->head = top;
	}

	free(body);
}

static void clawWait()
{
	self->face = self->head->face;
	
	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	
	self->startX = self->x;
	
	self->thinkTime++;
	
	self->x = self->startX + sin(DEG_TO_RAD(self->thinkTime)) * 16;
	
	self->y = self->head->y + self->offsetY + 128;
	
	self->startY = self->head->y + self->offsetY;
	
	checkToMap(self);
	
	alignArmToClaw();
	
	if (self->head->mental == -5)
	{
		self->targetX = self->face == LEFT ? self->startX - 64 : self->startX + 64;
		self->targetY = self->startY;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
		
		self->dirX *= 4;
		self->dirY *= 4;
		
		self->action = &punchAttackInit;
		
		self->thinkTime = self->layer == FOREGROUND_LAYER ? 30 : 60;
		
		self->maxThinkTime = 5;
	}
	
	else if (self->head->mental == -10)
	{
		/* Only one grab at a time */
		
		if ((self->head->endX == 2 && self->layer == FOREGROUND_LAYER) || self->head->endX == 1)
		{
			self->targetX = self->startX;
			self->targetY = self->startY;
			
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
			
			self->dirX *= 4;
			self->dirY *= 4;
			
			self->action = &grabAttackInit;
			
			self->thinkTime = 30;
		}
	}
}

static void punchAttackInit()
{
	if (atTarget())
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->targetX = self->face == LEFT ? self->startX - 256 : self->startX + 256;
			self->targetY = self->y;
			
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
			
			self->dirX *= 12;
			self->dirY *= 12;
			
			self->action = &punchAttack;
			
			self->reactToBlock = &punchReactToBlock;
		}
	}
	
	checkToMap(self);
	
	alignArmToClaw();
}

static void punchReactToBlock()
{
	self->maxThinkTime--;
	
	if (self->maxThinkTime <= 0)
	{
		self->targetX = self->startX;
		self->targetY = self->y;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
		
		self->dirX *= 4;
		self->dirY *= 4;
		
		self->action = &punchAttackFinish;
		
		self->thinkTime = 30;
	}
	
	else
	{
		self->targetX = self->face == LEFT ? self->startX - 64 : self->startX + 64;
		self->targetY = self->startY;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
		
		self->dirX *= 6;
		self->dirY *= 6;
		
		self->action = &punchAttackInit;
	}
}

static void punchAttack()
{
	if (atTarget())
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->maxThinkTime--;
			
			if (self->maxThinkTime <= 0)
			{
				self->targetX = self->startX;
				self->targetY = self->y;
				
				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
				
				self->dirX *= 4;
				self->dirY *= 4;
				
				self->action = &punchAttackFinish;
				
				self->thinkTime = 30;
			}
			
			else
			{
				self->targetX = self->face == LEFT ? self->startX - 64 : self->startX + 64;
				self->targetY = self->startY;
				
				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
				
				self->dirX *= 6;
				self->dirY *= 6;
				
				self->action = &punchAttackInit;
			}
		}
	}
	
	checkToMap(self);
	
	alignArmToClaw();
}

static void punchAttackFinish()
{
	if (atTarget())
	{
		self->targetX = self->startX;
		
		self->targetY = self->head->y + self->offsetY + 128;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
		
		self->dirX *= 4;
		self->dirY *= 4;
		
		self->action = &punchReturnToNormal;
	}
	
	checkToMap(self);
	
	alignArmToClaw();
}

static void punchReturnToNormal()
{
	if (atTarget())
	{
		self->thinkTime = 0;
		
		self->action = &clawWait;
		
		self->head->mental = 0;
		
		self->damage = 1;
		
		self->flags &= ~GRABBING;
		
		self->touch = &entityTouch;
	}
	
	checkToMap(self);
	
	alignArmToClaw();
}

static void grabAttackInit()
{
	if (atTarget())
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->targetX = player.x;
			self->targetY = self->y;
			
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
			
			self->dirX *= 12;
			self->dirY *= 12;
			
			self->action = &grabAttack;
			
			self->reactToBlock = NULL;
			
			self->touch = &grabTouch;
			
			self->damage = 0;
		}
	}
	
	checkToMap(self);
	
	alignArmToClaw();
}

static void grabAttack()
{
	if (atTarget() || self->dirX == 0)
	{
		if (self->flags & GRABBING)
		{
			self->touch = &entityTouch;
			
			self->action = &grabWait;
			
			self->endY = self->y - 16;
		}
		
		else
		{
			self->targetX = self->startX;
			self->targetY = self->y;
			
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
			
			self->dirX *= 12;
			self->dirY *= 12;
			
			self->action = &punchAttackFinish;
			
			self->thinkTime = 30;
		}
	}
	
	checkToMap(self);
	
	alignArmToClaw();
}

static void grabTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		self->thinkTime = 0;
		
		self->flags |= GRABBING;
		
		self->touch = &entityTouch;
	}
}

static void grabWait()
{
	self->thinkTime++;
	
	self->y = self->endY + sin(DEG_TO_RAD(self->thinkTime)) * 8;
	
	player.x = self->x + self->w / 2 - player.w / 2;
	player.y = self->y + self->h / 2 - player.h / 2;
	
	alignArmToClaw();
	
	if (self->thinkTime >= 420)
	{
		self->targetX = self->x + (self->face == LEFT ? -128 : 128);
		self->targetY = self->startY - 64;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
		
		self->dirX *= 4;
		self->dirY *= 4;
		
		self->action = &dropPlayer;
		
		self->thinkTime = 30;
	}
}

static void dropPlayer()
{
	if (atTarget())
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->flags &= ~GRABBING;
			
			self->targetX = self->startX;
			self->targetY = self->startY;
			
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
			
			self->dirX *= 12;
			self->dirY *= 12;
			
			self->action = &punchAttackFinish;
			
			self->thinkTime = 30;
		}
	}
	
	alignArmToClaw();
	
	checkToMap(self);
	
	if (self->flags & GRABBING)
	{
		player.x = self->x + self->w / 2 - player.w / 2;
		player.y = self->y + self->h / 2 - player.h / 2;
	}
}

static void armPartWait()
{
	int x, y;
	Entity *e;
	
	self->face = self->head->face;
	
	self->damage = self->head->damage;

	if (prand() % 120 == 0)
	{
		x = self->x + self->w / 2 + ((prand() % 6) * (prand() % 2 == 0 ? -1 : 1));
		y = self->y + self->h - prand() % 10;

		e = addProjectile("enemy/slime_drip", self, x, y, 0, 0);

		e->x -= e->w / 2;

		e->touch = NULL;
	}
	
	if (self->head->health <= 0)
	{
		self->dirX = 0;
		self->dirY = 0;
		
		self->action = &entityDieNoDrop;
	}
}

static void alignArmToClaw()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->x;
	y = self->y;

	partDistanceX = self->startX - self->x;
	partDistanceY = self->startY - self->y;

	partDistanceX /= self->mental;
	partDistanceY /= self->mental;

	e = self->target;

	while (e != NULL)
	{
		x += partDistanceX;
		y += partDistanceY;

		e->x = (e->target == NULL ? self->startX : x) + (self->w - e->w) / 2;
		e->y = (e->target == NULL ? self->startY : y);

		e->damage = self->damage;

		e->face = self->face;

		if (self->flags & FLASH)
		{
			e->flags |= FLASH;
		}

		else
		{
			e->flags &= ~FLASH;
		}

		e = e->target;
	}
}

static void hover()
{
	return;
	
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 16;
}

static void slimeAttackInit()
{
	Target *t = getTargetByName("SEWER_BOSS_SLIME_TARGET");
	
	if (t == NULL)
	{
		showErrorAndExit("Sewer Boss cannot find target");
	}
	
	self->targetX = self->x;
	self->targetY = t->y;
	
	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
	
	self->dirX *= 1;
	self->dirY *= 1;
	
	self->action = &slimeAttackMoveToTarget;
}

static void slimeAttackMoveToTarget()
{
	if (atTarget())
	{
		setEntityAnimation(self, ATTACK_1);
		
		self->animationCallback = &slimeAttack;
		
		self->mental = 5;
		
		self->thinkTime = 0;
	}
	
	checkToMap(self);
}

static void slimeAttack()
{
	self->action = &slimeAttack;
	
	setEntityAnimation(self, ATTACK_2);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->mental--;
		
		if (self->mental <= 0)
		{
			self->action = &slimeAttackMouthClose;
		}
		
		else
		{
			self->thinkTime = 30;
		}
	}
}

static void slimeAttackMouthClose()
{
	setEntityAnimation(self, ATTACK_3);
	
	self->animationCallback = &slimeAttackFinish;
	
	self->targetX = self->startX;
	self->targetY = self->startY;
	
	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
	
	self->dirX *= 2;
	self->dirY *= 2;
}

static void slimeAttackFinish()
{
	self->action = &slimeAttackFinish;
	
	if (atTarget())
	{
		self->action = &attackFinished;
	}
	
	setEntityAnimation(self, STAND);
	
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (strcmpignorecase(other->name, "item/ice_cube") == 0 || strcmpignorecase(other->name, "item/ice_platform") == 0)
	{
		other->inUse = FALSE;
	}
	
	else
	{
		entityTouch(other);
	}
}

static void clawTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
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

				enemyPain();
			}

			else
			{
				self->head->endX--;
				
				self->head->mental = 0;
				
				self->damage = 0;

				if (other->type == WEAPON || other->type == PROJECTILE)
				{
					increaseKillCount();
				}

				self->die();
			}
		}
	}
}
