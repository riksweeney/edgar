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
#include "../item/item.h"
#include "../item/exploding_gayzer_eye.h"
#include "../item/glass_cage.h"
#include "../projectile.h"
#include "../inventory.h"

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
static void swordReactToBlock(void);
static int bombOnScreen(void);
static void crusherWait(void);
static void bowAttackInit(void);
static void bowAttack(void);
static void fireArrowWait(void);
static void fireArrowFinish(void);
static void fireArrow(void);
static void attackOnPlatformInit(void);
static void takeDamage(Entity *, int);
static void healSelf(void);
static void activate(int);
static void stunned(void);
static void getClosestTarget(void);

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
			self->activate = &activate;
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
		
		self->takeDamage = &takeDamage;
		
		self->target = getEntityByObjectiveName("LAB_CRUSHER");
		
		if (self->target == NULL)
		{
			showErrorAndExit("Evil Edgar cannot find LAB_CRUSHER");
		}
		
		addSwordSwing();
		
		playBossMusic("music/battle_for_life.xm");

		initBossHealthBar();
		
		self->damage = 1;
		
		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	self->mental = 0;
	
	self->thinkTime = 60;
	
	self->action = &healSelf;
	
	checkToMap(self);
}

static void healSelf()
{
	if (self->health != self->maxHealth)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			if (prand() % 10 == 0)
			{
				setInfoBoxMessage(60, 255, 255, 255, _("He is using that machine to heal himself..."));
			}
			
			self->health = self->maxHealth;
		}
	}
	
	else
	{
		self->thinkTime = 30;
		
		self->action = &wait;
	}
	
	checkToMap(self);
}

static void wait()
{
	int r;
	
	if (player.health > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			getClosestTarget();
			
			if (bombOnScreen() == TRUE)
			{
				self->thinkTime = 30;
			}
			
			else if (player.y < self->targetY)
			{
				self->action = &attackOnPlatformInit;
			}
			
			else
			{
				r = prand() % 3;
				
				switch (r)
				{
					case 0:
						self->thinkTime = 60;
						
						self->dirX = self->face == LEFT ? -self->speed : self->speed;
						
						setEntityAnimation(self, CUSTOM_1);
						
						self->action = &bowAttackInit;
					break;
					
					case 1:
						self->action = &crusherWait;
						
						self->target->active = TRUE;
						
						self->target->mental = 3 + prand() % 3;
					break;
					
					default:
						self->thinkTime = 60;
						
						self->action = &attackPlayerInit;
						
						self->dirX = self->face == LEFT ? -self->speed : self->speed;
						
						setEntityAnimation(self, WALK);
					break;
				}
			}
		}
	}
	
	else if (self->mental != -1)
	{
		if (getInventoryItem("Amulet of Resurrection") == NULL)
		{
			if (player.flags |= NO_DRAW)
			{
				createAutoDialogBox(_("Evil_Edgar"), _("What a mess..."), 60);
			}
			
			else
			{
				createAutoDialogBox(_("Evil_Edgar"), _("I'm better than you in every single way..."), 60);
			}
			
			self->mental = -1;
		}
	}
	
	checkToMap(self);
}

static void attackOnPlatformInit()
{
	self->endX = 2;
	
	createAutoDialogBox(_("Evil_Edgar"), _("Get down from there!"), 60);
	
	self->mental = 5;
	
	self->action = &bowAttack;
	
	checkToMap(self);
}

static void bowAttackInit()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->mental = 3 + prand() % 4;
		
		self->action = &bowAttack;
	}
	
	checkToMap(self);
}

static void bowAttack()
{
	if (!(self->flags & ON_GROUND))
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
	
	else
	{
		self->dirX = 0;
		
		facePlayer();
		
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, ATTACK_2);
			
			self->animationCallback = &fireArrow;
			
			self->thinkTime = 15;
			
			self->action = &fireArrowWait;
		}
	}
	
	checkToMap(self);
}

static void fireArrowWait()
{
	checkToMap(self);
}

static void fireArrow()
{
	Entity *e;
	
	e = addProjectile(self->health < 750 ? "weapon/flaming_arrow" : "weapon/normal_arrow", self, self->x + (self->face == RIGHT ? 0 : self->w), self->y + 27, self->face == RIGHT ? 12 : -12, 0);
	
	e->damage = 1;
	
	if (e->face == LEFT)
	{
		e->x -= e->w;
	}
	
	playSoundToMap(self->health < 750 ? "sound/enemy/fireball/fireball.ogg" : "sound/edgar/arrow.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	e->reactToBlock = &bounceOffShield;

	e->face = self->face;

	e->flags |= FLY;
	
	setEntityAnimation(self, ATTACK_3);
	
	self->animationCallback = &fireArrowFinish;
	
	checkToMap(self);
}

static void fireArrowFinish()
{
	self->action = &fireArrowFinish;
	
	setEntityAnimation(self, CUSTOM_2);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->mental--;
		
		if (bombOnScreen() == TRUE)
		{
			self->action = &attackPlayer;
			
			self->thinkTime = 0;
		}
		
		else
		{
			self->action = self->mental <= 0 ? &attackPlayer : &bowAttack;
		}
	}
	
	checkToMap(self);
}

static void attackPlayerInit()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->endY = 1 + prand() % 3;
		
		self->thinkTime = 300;
		
		self->action = &attackPlayer;
	}
	
	checkToMap(self);
}

static void crusherWait()
{
	if (self->target->active == FALSE || player.health <= 0)
	{
		self->target->mental = 0;
		
		self->target->active = FALSE;
		
		self->endX = 0;
		
		self->action = &attackFinished;
	}
	
	checkToMap(self);
}

static void attackPlayer()
{
	if (!(self->flags & ON_GROUND))
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
	
	else
	{
		if (prand() % 5 == 0 && bombOnScreen() == TRUE)
		{
			self->thinkTime = 0;
		}
		
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			if (self->endX == 2)
			{
				self->endX = 0;
				
				self->action = &attackFinished;
			}
			
			else
			{
				getClosestTarget();
				
				setEntityAnimation(self, WALK);
				
				self->action = &goToTarget;
			}
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
		self->thinkTime = 60;
		
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
			self->endY--;
			
			if (self->endY <= 0)
			{
				self->thinkTime = 0;
			}
			
			else
			{
				self->thinkTime = 30;
			}
			
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
		
		if (self->y < self->targetY)
		{
			self->action = &attackFinished
		}
		
		else if (prand() % 2 == 0)
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
		
		self->reactToBlock = &swordReactToBlock;
		
		setEntityAnimation(self, ATTACK_1);
		
		self->animationCallback = &swordSwingAttackFinish;
		
		playSoundToMap("sound/edgar/swing.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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
	
	if (getInventoryItem("Exploding Gayzer Eye") == NULL && prand() % 3 == 0)
	{
		e = addExplodingGayzerEye(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/exploding_gayzer_eye_dud");
		
		e->head = self;
		
		self->mental = 2;
	}
	
	else
	{
		e = addExplodingGayzerEye(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/exploding_gayzer_eye");
	}
	
	e->dirX = self->face == LEFT ? -(5 + prand() % 10) : 5 + prand() % 10;
	e->dirY = -12;
	
	e->thinkTime = 60;

	playSoundToMap("sound/common/throw.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

	self->thinkTime = 60;
	
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
					createAutoDialogBox(_("Evil_Edgar"), _("Gotcha!"), 60);
					
					self->action = &activateGlassCage;
				}
				
				else
				{
					self->action = &attackFinished;
				}
			}
			
			else
			{
				if (self->mental == 2)
				{
					createAutoDialogBox(_("Evil_Edgar"), _("Stupid duds..."), 60);
				}
				
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
	
	self->thinkTime = 300;
}

static void cageWait()
{
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		createAutoDialogBox(_("Evil_Edgar"), _("Die!"), 60);
		
		self->action = &crusherWait;
		
		self->target->active = TRUE;
		
		self->target->mental = 1;
	}
	
	else if (self->mental == 0)
	{
		self->action = &attackFinished;
	}
}

static void swordReactToBlock()
{
	self->damage = 0;
}

static int bombOnScreen()
{
	int onScreen = FALSE;
	
	onScreen = getEntityByName("item/bomb") == NULL ? FALSE : TRUE;
	
	if (onScreen == FALSE)
	{
		onScreen = getEntityByName("common/explosion") == NULL ? FALSE : TRUE;
	}
	
	if (onScreen == TRUE)
	{
		createAutoDialogBox(_("Evil_Edgar"), _("I'm not falling for that!"), 60);
	}
	
	return onScreen;
}

static void takeDamage(Entity *other, int damage)
{
	int health;
	Entity *e;
	
	if (!(self->flags & INVULNERABLE))
	{
		health = self->health;
		
		self->health -= damage;
		
		if (health >= 1250 && self->health < 1250)
		{
			e = addPermanentItem("item/darsh_fat", self->x, self->y);
			
			e->dirX = self->face == LEFT ? 8 : 8;
			e->dirY = -8;
		}
		
		else if (health >= 1000 && self->health < 1000)
		{
			e = addPermanentItem("item/condor_bones", self->x, self->y);
			
			e->dirX = self->face == LEFT ? 8 : 8;
			e->dirY = -8;
		}
		
		else if (health >= 750 && self->health < 750)
		{
			e = addPermanentItem("item/condor_bones", self->x, self->y);
			
			e->dirX = self->face == LEFT ? 8 : 8;
			e->dirY = -8;
		}
		
		else if (self->health <= 0)
		{
			self->health = 0;
		}
		
		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		
		enemyPain();
	}
}

static void activate(int val)
{
	if (val == 100)
	{
		createAutoDialogBox(_("Evil_Edgar"), _("Arghh! My eyes!"), 60);
		
		setEntityAnimation(self, DIE);
		
		self->thinkTime = 600;
		
		self->action = &stunned;
	}
}

static void stunned()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		getClosestTarget();
		
		self->action = &goToTarget;
	}
	
	checkToMap(self);
}

static void getClosestTarget()
{
	Target *t;
	
	t = getTargetByName(self->x < player.x ? "EVIL_EDGAR_TARGET_LEFT" : "EVIL_EDGAR_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Evil Edgar cannot find target");
	}
	
	self->targetX = t->x;
	self->targetY = t->y;
}
