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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../audio/music.h"
#include "../player.h"
#include "../hud.h"
#include "../map.h"
#include "../dialog.h"
#include "../graphics/gib.h"
#include "../system/error.h"
#include "../world/target.h"
#include "../item/exploding_gayzer_eye.h"
#include "../item/glass_cage.h"

extern Entity *self, player;

static void init(void);
static void fadeIn(void);
static void cutsceneWait(void);
static void introWait(void);
static void attackFinished(void);
static void attackPlayer(void);
static void goToTarget(void);
static void jumpUp(void);
static void wait(void);
static void throwGayzerEye(void);
static void throwWait(void);
static void activateGlassCage(void);
static void cageWait(void);
static void attackPlayerInit(void);
static void attackPlayer(void);
static void slashInit(void);
static void slash(void);
static void addSwordSwing(void);
static void swordSwingWait(void);
static void swordSwingAttack(void);
static void swordSwingAttackFinish(void);

Entity *addEvilEdgar(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Evil Edgar");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->alpha = 0;
			self->action = &fadeIn;
		break;
		
		case 3:
			self->action = &introWait;
		break;
		
		default:
			self->action = &cutsceneWait;
		break;
	}
}

static void introWait()
{
	Entity *temp;
	
	self->flags &= ~NO_DRAW;
	
	if (self->active == TRUE)
	{
		temp = self;
		
		setContinuePoint(FALSE, self->name, NULL);
		
		self = temp;
		
		self->touch = &entityTouch;
		
		addSwordSwing();
		
		playBossMusic("music/battle_for_life.xm");

		initBossHealthBar();
		
		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	self->mental = 0;
	
	self->thinkTime = 120;
	
	self->action = &wait;
	
	checkToMap(self);
}

static void wait()
{
	if (player.health > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->thinkTime = 180 + (prand() % 180);
			
			self->action = &attackPlayerInit;
			
			self->dirX = self->face == LEFT ? -self->speed : self->speed;
			
			setEntityAnimation(self, WALK);
		}
	}
	
	checkToMap(self);
}

static void attackPlayerInit()
{
	checkToMap(self);
	
	if (!(self->flags & ON_GROUND))
	{
		self->action = &attackPlayer;
	}
}

static void attackPlayer()
{
	Target *t;
	
	if (!(self->flags & ON_GROUND))
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
	
	else
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			t = getTargetByName(self->x < player.x ? "EVIL_EDGAR_TARGET_LEFT" : "EVIL_EDGAR_TARGET_RIGHT");
			
			if (t == NULL)
			{
				showErrorAndExit("Evil Edgar cannot find target");
			}
			
			self->targetX = t->x;
			
			setEntityAnimation(self, WALK);
			
			self->action = &goToTarget;
		}
		
		else
		{
			/* Get close to the player */
			
			facePlayer();
			
			if ((self->face == LEFT && abs(self->x - (player.x + player.w)) < 16) || (self->face == RIGHT && abs(player.x - (self->x + self->w)) < 16))
			{
				setEntityAnimation(self, STAND);
				
				self->dirX = 0;
				
				self->mental = 3;
				
				self->thinkTime = 30;
				
				self->action = &slashInit;
			}
			
			else
			{
				self->dirX = self->face == LEFT ? -self->speed : self->speed;
				
				setEntityAnimation(self, WALK);
			}
		}
	}
	
	checkToMap(self);
}

static void slashInit()
{
	self->thinkTime--;
	
	if (self->thinkTime == 0)
	{
		self->thinkTime = 120;
		
		self->action = &slash;
		
		setEntityAnimation(self, ATTACK_1);
		
		self->flags |= ATTACKING;
	}
	
	checkToMap(self);
}

static void slash()
{	
	if (self->mental == 0)
	{
		setEntityAnimation(self, STAND);
		
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->thinkTime = 0;
			
			self->action = &attackPlayer;
		}
	}
	
	checkToMap(self);
}

static void goToTarget()
{
	if (fabs(self->x - self->targetX) <= self->speed)
	{
		self->dirX = 0;
		
		facePlayer();
		
		setEntityAnimation(self, STAND);
		
		if (prand() % 4 == 0)
		{
			self->dirY = -20;
			
			self->thinkTime = 60;
			
			self->action = &jumpUp;
		}
		
		else
		{
			self->mental = 1;
			
			self->action = &throwGayzerEye;
		}
	}
	
	else
	{
		self->face = self->targetX < self->x ? LEFT : RIGHT;
		
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
	
	checkToMap(self);
}

static void addSwordSwing()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Evil Edgar Sword");
	}

	loadProperties("boss/evil_edgar_sword", e);

	e->x = 0;
	e->y = 0;

	e->action = &swordSwingWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;
	
	e->head = self;
	
	e->flags |= ATTACKING;

	setEntityAnimation(e, STAND);
}

static void swordSwingWait()
{
	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
	
	if (self->head->flags & ATTACKING)
	{
		self->action = &swordSwingAttack;
		
		setEntityAnimation(self, ATTACK_1);
		
		self->animationCallback = &swordSwingAttackFinish;
	}
	
	else
	{
		self->flags |= NO_DRAW;
	}
}

static void swordSwingAttack()
{
	self->flags &= ~NO_DRAW;
	
	self->damage = 1;
	
	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void swordSwingAttackFinish()
{
	self->damage = 0;
	
	self->flags |= NO_DRAW;
	
	setEntityAnimation(self, STAND);
	
	self->action = &swordSwingWait;
	
	self->head->mental--;
	
	if (self->head->mental == 0)
	{
		self->head->flags &= ~ATTACKING;
	}
}

static void throwGayzerEye()
{
	Entity *e;
	
	e = addExplodingGayzerEye(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/exploding_gayzer_eye");
	
	e->dirX = self->face == LEFT ? -(5 + prand() % 10) : 5 + prand() % 10;
	e->dirY = -12;

	playSoundToMap("sound/common/throw.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

	self->thinkTime = 30;
	
	self->action = &throwWait;
	
	checkToMap(self);
}

static void throwWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirY = -20;
		
		self->thinkTime = 90;
		
		self->action = &jumpUp;
	}
	
	checkToMap(self);
}

static void jumpUp()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			if (self->mental == 1)
			{
				if (player.flags & HELPLESS)
				{
					createAutoDialogBox(_("Evil Edgar"), _("Got you!"), 60);
					
					self->action = &activateGlassCage;
				}
				
				else
				{
					self->action = &attackFinished;
				}
			}
			
			else
			{
				self->action = &attackFinished;
			}
		}
	}
}

static void fadeIn()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
		
		if (self->mental == 0)
		{
			self->thinkTime--;
			
			if (self->thinkTime <= 0)
			{
				self->alpha++;
				
				self->thinkTime = 1;
				
				if (self->alpha >= 255)
				{
					self->alpha = 255;
					
					self->mental = 1;
				}
			}
		}
	}
	
	checkToMap(self);
}

static void cutsceneWait()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
		
		self->touch = &entityTouch;
	}
	
	checkToMap(self);
}

static void activateGlassCage()
{
	Entity *e = addGlassCage(0, 0, "item/glass_cage");
	
	e->x = player.x + player.w / 2 - e->w / 2;
	e->y = getCameraMaxY();
	
	e->active = FALSE;
	
	e->startY = e->y - e->h;
	
	e->head = self;
	
	e->target = &player;
	
	self->action = &cageWait;
}

static void cageWait()
{
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}
}
