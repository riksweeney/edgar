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
#include "../credits.h"
#include "../custom_actions.h"
#include "../dialog.h"
#include "../entity.h"
#include "../event/script.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/bomb.h"
#include "../item/exploding_gazer_eye.h"
#include "../item/exploding_gazer_eye_dud.h"
#include "../item/glass_cage.h"
#include "../item/item.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"

extern Entity *self, player;

static void init(void);
static void fadeIn(void);
static void cutsceneWait(void);
static void introWait(void);
static void attackFinished(void);
static void attackPlayer(void);
static void goToTarget(void);
static void jumpUp(void);
static void entityWait(void);
static void throwGazerEye(void);
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
static void swordReactToBlock(Entity *);
static int bombOnScreen(void);
static void crusherWait(void);
static void bowAttackInit(void);
static void bowAttack(void);
static void fireArrowWait(void);
static void fireArrowFinish(void);
static void fireArrow(void);
static void attackOnPlatformInit(void);
static void takeDamage(Entity *, int);
static void activate(int);
static void stunned(void);
static void getClosestTarget(void);
static void gotoTargetMachineInit(void);
static void gotoTargetMachine(void);
static int edgarInSourceMachine(void);
static void die(void);
static void throwMiniBomb(void);
static void healSelfInit(void);
static void healSelf(void);
static void potionWait(void);

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

	e->creditsAction = &bossMoveToMiddle;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

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
			self->die = &die;
		break;

		default:
			self->action = &cutsceneWait;
		break;
	}
}

static void introWait()
{
	self->flags &= ~NO_DRAW;

	if (self->active == TRUE)
	{
		setContinuePoint(FALSE, self->name, NULL);

		self->touch = &entityTouch;

		self->takeDamage = &takeDamage;

		addSwordSwing();

		playBossMusic("EVIL_EDGAR_MUSIC");

		initBossHealthBar();

		self->damage = 1;

		self->startX = 0;

		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	self->mental = 0;

	self->thinkTime = 30;

	self->action = &entityWait;

	checkToMap(self);
}

static void entityWait()
{
	int random;

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

			else if (self->startX == 1 && self->health < self->maxHealth / 2)
			{
				self->action = &healSelfInit;
			}

			else if (edgarInSourceMachine() == TRUE)
			{
				createAutoDialogBox(_("Evil_Edgar"), _("You got into the machine by yourself?! You're even stupider than I thought you were!"), 180);

				self->thinkTime = 60;

				self->action = &gotoTargetMachineInit;
			}

			else if (player.y < self->targetY)
			{
				self->action = &attackOnPlatformInit;
			}

			else
			{
				random = self->startX == 1 ? prand() % 4 : prand() % 3;

				switch (random)
				{
					case 0:
						self->thinkTime = 60;

						self->dirX = self->face == LEFT ? -self->speed : self->speed;

						setEntityAnimation(self, "CUSTOM_1");

						self->action = &bowAttackInit;
					break;

					case 1:
						self->thinkTime = 60;

						self->action = &attackPlayerInit;

						self->dirX = self->face == LEFT ? -self->speed : self->speed;

						setEntityAnimation(self, "WALK");
					break;

					case 2:
						self->action = &crusherWait;

						self->target = getEntityByObjectiveName("LAB_CRUSHER");

						if (self->target == NULL)
						{
							showErrorAndExit("Evil Edgar cannot find LAB_CRUSHER");
						}

						self->target->active = TRUE;

						self->target->mental = 3 + prand() % 3;
					break;

					default:
						self->mental = 5 + prand() % 6;

						self->action = &throwMiniBomb;

						setEntityAnimation(self, "STAND");
					break;
				}
			}
		}
	}

	else if (self->mental != -1)
	{
		if (getInventoryItemByObjectiveName("Amulet of Resurrection") == NULL)
		{
			createAutoDialogBox(_("Evil_Edgar"), _("I'm better than you in every single way..."), 60);

			self->mental = -1;
		}
	}

	checkToMap(self);
}

static void attackOnPlatformInit()
{
	self->endX = 2;

	createAutoDialogBox(_("Evil_Edgar"), _("Get down from there!"), 60);

	self->mental = 10;

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
			setEntityAnimation(self, "ATTACK_2");

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

	e = addProjectile(self->startX == 1 ? "weapon/flaming_arrow" : "weapon/normal_arrow", self, self->x + (self->face == RIGHT ? 0 : self->w), self->y + self->offsetY, self->face == RIGHT ? 12 : -12, 0);

	e->damage = 1;

	if (e->face == LEFT)
	{
		e->x -= e->w;
	}

	playSoundToMap(self->startX == 1 ? "sound/enemy/fireball/fireball" : "sound/edgar/arrow", BOSS_CHANNEL, self->x, self->y, 0);

	e->reactToBlock = &bounceOffShield;

	e->face = self->face;

	e->flags |= FLY;

	setEntityAnimation(self, "ATTACK_3");

	self->animationCallback = &fireArrowFinish;

	checkToMap(self);
}

static void fireArrowFinish()
{
	self->action = &fireArrowFinish;

	setEntityAnimation(self, "CUSTOM_2");

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

				setEntityAnimation(self, "WALK");

				self->action = &goToTarget;
			}
		}

		else
		{
			/* Get close to the player */

			facePlayer();

			if ((self->face == LEFT && abs(self->x - (player.x + player.w)) < 16) || (self->face == RIGHT && abs(player.x - (self->x + self->w)) < 16))
			{
				setEntityAnimation(self, "STAND");

				self->dirX = 0;

				self->mental = 3;

				self->thinkTime = 30;

				self->action = &slashInit;
			}

			else
			{
				self->dirX = self->face == LEFT ? -self->speed : self->speed;

				setEntityAnimation(self, "WALK");
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

		setEntityAnimation(self, "ATTACK_1");

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void slash()
{
	if (self->mental == 0)
	{
		setEntityAnimation(self, "STAND");

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

		setEntityAnimation(self, "STAND");

		if (self->y < self->targetY)
		{
			self->action = &attackFinished;
		}

		else
		{
			if (prand() % 2 == 0)
			{
				self->dirY = -20;

				self->thinkTime = 60;

				self->action = &jumpUp;
			}

			else
			{
				self->mental = 1;

				self->action = &throwGazerEye;
			}
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

	setEntityAnimation(e, "STAND");
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

		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &swordSwingAttackFinish;

		playSoundToMap("sound/edgar/swing", BOSS_CHANNEL, self->x, self->y, 0);
	}

	else
	{
		self->damage = 0;

		self->flags |= NO_DRAW;
	}

	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
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

	setEntityAnimation(self, "STAND");

	self->action = &swordSwingWait;

	self->head->mental--;

	if (self->head->mental == 0)
	{
		self->head->flags &= ~ATTACKING;
	}
}

static void throwGazerEye()
{
	Entity *e;

	if (getInventoryItemByObjectiveName("Dud Exploding Gazer Eyeball") == NULL && getEntityByName("item/exploding_gazer_eye_dud") == NULL)
	{
		if (prand() % 3 == 0)
		{
			e = addExplodingGazerEyeDud(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/exploding_gazer_eye_dud");

			e->head = self;

			e->active = FALSE;

			self->mental = 2;
		}

		else
		{
			e = addExplodingGazerEye(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/exploding_gazer_eye");
		}
	}

	else
	{
		e = addExplodingGazerEye(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/exploding_gazer_eye");
	}

	e->dirX = self->face == LEFT ? -(5 + prand() % 10) : 5 + prand() % 10;
	e->dirY = -12;

	e->thinkTime = 60;

	playSoundToMap("sound/common/throw", EDGAR_CHANNEL, player.x, player.y, 0);

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
				if ((player.flags & HELPLESS) && edgarInSourceMachine() == FALSE)
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
	int min, max;
	Entity *e = addGlassCage(0, 0, "item/glass_cage");

	min = getCameraMinX();
	max = getCameraMaxX();

	e->x = player.x + player.w / 2 - e->w / 2;
	e->y = getCameraMaxY();

	if (e->x < min)
	{
		e->x = min;
	}

	else if (e->x + e->w >= max)
	{
		e->x = max - e->w - 1;
	}

	e->active = FALSE;

	e->health = 480;

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
		self->target = getEntityByObjectiveName("LAB_CRUSHER");

		if (self->target == NULL)
		{
			showErrorAndExit("Evil Edgar cannot find LAB_CRUSHER");
		}

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

static void swordReactToBlock(Entity *other)
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

	return onScreen;
}

static void takeDamage(Entity *other, int damage)
{
	int health, halfHealth;
	Entity *e, *temp;

	if (!(self->flags & INVULNERABLE))
	{
		health = self->health;

		halfHealth = self->maxHealth / 2;

		/* Take minimal damage from bombs */

		if (other->type == EXPLOSION)
		{
			damage = 1;
		}

		self->health -= damage;

		if (self->startX == 0)
		{
			if (health >= halfHealth && self->health < halfHealth)
			{
				e = addPermanentItem("item/spanner", self->x, self->y);

				e->dirX = self->face == LEFT ? -(5 + prand() % 10) : 5 + prand() % 10;
				e->dirY = -12;

				setCustomAction(e, &invulnerable, 90, 0, 0);

				self->startX = 1;
			}
		}

		if (self->health <= 1)
		{
			self->health = 1;
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();
	}
}

static void activate(int val)
{
	if (val == 100)
	{
		createAutoDialogBox(_("Evil_Edgar"), _("Arghh! My eyes!"), 120);

		self->animationCallback = NULL;

		setEntityAnimation(self, "DIE");

		self->thinkTime = 600;

		self->action = &stunned;

		self->dirX = 0;

		self->mental = 0;

		self->flags &= ~ATTACKING;
	}
}

static void stunned()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		getClosestTarget();

		setEntityAnimation(self, "WALK");

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

static int edgarInSourceMachine()
{
	Entity *machine = getEntityByObjectiveName("SOURCE_MACHINE");

	if (machine == NULL)
	{
		showErrorAndExit("Evil Edgar cannot find SOURCE_MACHINE");
	}

	return machine->health == 2 ? TRUE : FALSE;
}

static void gotoTargetMachineInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "WALK");

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->thinkTime = 60;

		self->target = getEntityByObjectiveName("TARGET_MACHINE");

		self->targetX = self->target->x + self->target->w / 2 - self->w / 2;

		self->action = &gotoTargetMachine;
	}

	checkToMap(self);
}

static void gotoTargetMachine()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->flags & ON_GROUND)
		{
			if (fabs(self->x - self->targetX) <= self->speed)
			{
				setEntityAnimation(self, "STAND");

				self->dirX = 0;

				self->action = &doNothing;

				runScript("soul_merger");
			}

			else
			{
				self->dirX = self->targetX < self->x ? -self->speed : self->speed;

				self->face = self->dirX < 0 ? LEFT : RIGHT;
			}
		}
	}

	checkToMap(self);
}

static void die()
{
	clearContinuePoint();

	increaseKillCount();

	freeBossHealthBar();

	self->inUse = FALSE;
}

static void throwMiniBomb()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addMiniBomb(self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, "item/bomb");

		e->dirX = self->face == LEFT ? -(3 + prand() % 10) : 3 + prand() % 10;
		e->dirY = -12;

		e->flags |= BOUNCES;

		playSoundToMap("sound/common/throw", BOSS_CHANNEL, self->x, self->y, 0);

		self->thinkTime = 30;

		self->mental--;

		e->endX = playSoundToMap("sound/item/fuse", 3, self->x, self->y, -1);

		if (self->mental <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void healSelfInit()
{
	Entity *e;

	e = addPermanentItem("item/health_potion", self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/throw", BOSS_CHANNEL, self->x, self->y, 0);

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->touch = NULL;

	e->action = &potionWait;

	e->head = self;

	e->dirY = ITEM_JUMP_HEIGHT;

	setCustomAction(e, &invulnerable, 90, 0, 0);

	self->action = &healSelf;

	checkToMap(self);
}

static void potionWait()
{
	if (!(self->flags & INVULNERABLE))
	{
		self->head->health = self->head->maxHealth;

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void healSelf()
{
	if (self->health == self->maxHealth)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}
