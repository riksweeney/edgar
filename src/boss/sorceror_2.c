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
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../enemy/thunder_cloud.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"
#include "sorceror.h"

extern Entity *self, player, playerWeapon, playerShield;

static void init(void);
static void entityWait(void);
static void attackFinished(void);
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
static void createShieldFinish(void);
static void pieceMoveToShield(void);
static void shieldCreateWait(void);
static void shieldWait(void);
static void shieldTakeDamage(Entity *, int);
static void shieldDie(void);
static void pieceWait(void);
static void holdPersonBackWait(void);
static void holdPersonPieceMove(void);
static void castLightningBolt(void);
static void lightningBolt(void);
static void flameWaveDropInit(void);
static void flameWaveDrop(void);
static void flameWaveAttack(void);
static void flameWaveMove(void);
static void flameWaveAnim(void);
static void teleportAway(void);
static void teleportAwayFinish(void);
static void takeDamage(Entity *, int);
static void callSummonersInit(void);
static void callSummoners(void);
static void callSummonersWait(void);
static void plasmaAttackInit(void);
static void plasmaAttack(void);
static void plasmaAttackFinish(void);
static void plasmaTakeDamage(Entity *, int);
static void disintegrationAttackInit(void);
static void disintegrationAttack(void);
static void disintegrationSpellInit(void);
static void disintegrationSpellAttack(void);
static void disintegrationAttackWait(void);
static void disintegrationTouch(Entity *);
static void riftAttackInit(void);
static void riftAttack(void);
static void riftAttackWait(void);
static void riftWait(void);
static void riftMove(void);
static void riftClose(void);
static void addRiftEnergy(int, int);
static void energyMoveToRift(void);
static void destroyFloorInit(void);
static void destroyFloor(void);
static void continuePoint(void);
static void hover(void);
static void finalAttackInit(void);
static void finalAttack(void);
static void finalSpellWait(void);
static void finalAttackFinish(void);
static void staffMoveAroundSorceror(void);
static void die(void);
static void dieWait(void);

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

	e->takeDamage = &takeDamage;

	e->die = &die;

	e->creditsAction = &bossMoveToMiddle;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		initBossHealthBar();

		setContinuePoint(FALSE, self->name, &continuePoint);

		self->thinkTime = 120;

		self->action = &teleportAway;

		self->maxThinkTime = 0;

		self->startX = 0;

		self->startY = -1;
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->dirX = 0;

	self->dirY = 0;

	self->mental = 0;

	self->thinkTime = 120;

	self->action = &entityWait;

	setEntityAnimation(self, "STAND");
}

static void entityWait()
{
	int rand;
	Entity *disintegrationShield;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->startX == 0)
		{
			switch ((int)self->startY)
			{
				case 0:
					self->action = &callSummonersInit;
				break;

				case 3:
					self->action = &disintegrationAttackInit;
				break;

				default:
					self->action = &createShieldInit;
				break;
			}
		}

		else
		{
			disintegrationShield = getInventoryItemByObjectiveName("Disintegration Shield");

			if (disintegrationShield == NULL)
			{
				disintegrationShield = getEntityByObjectiveName("Disintegration Shield");
			}

			switch (disintegrationShield->health)
			{
				case 1:
					rand = prand() % 3;
				break;

				case 2:
					rand = prand() % 5;
				break;

				case 3:
					rand = prand() % 6;
				break;

				default:
					rand = 6;
				break;
			}

			if (getEntityByName("enemy/flame_statue") != NULL)
			{
				switch (rand)
				{
					case 0:
						self->action = &plasmaAttackInit;
					break;

					case 1:
						self->action = &flameWaveDropInit;
					break;

					case 2:
						self->action = &destroyFloorInit;
					break;

					case 3:
						self->action = &riftAttackInit;
					break;

					case 6:
						self->action = &finalAttackInit;
					break;

					default:
						self->action = &holdPersonInit;
					break;
				}
			}

			else
			{
				switch (rand)
				{
					case 0:
						self->action = &plasmaAttackInit;
					break;

					case 1:
						self->action = &flameWaveDropInit;
					break;

					case 2:
						self->action = &destroyFloorInit;
					break;

					case 3:
						self->action = &riftAttackInit;
					break;

					case 4:
						self->action = &holdPersonInit;
					break;

					case 6:
						self->action = &finalAttackInit;
					break;

					default:
						self->action = &statueAttackInit;
					break;
				}
			}
		}
	}

	hover();
}

static void finalAttackInit()
{
	Target *t;

	t = getTargetByName("SORCEROR_MID_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->flags |= NO_DRAW;

	setEntityAnimation(self, "ATTACK_4");

	self->x = t->x;
	self->y = t->y;

	self->thinkTime = 30;

	self->action = &finalAttack;

	self->mental = 0;

	hover();
}

static void finalAttack()
{
	Entity *temp;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		switch (self->mental)
		{
			case 0:
				addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

				playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

				self->flags &= ~NO_DRAW;

				self->face = RIGHT;

				createAutoDialogBox(_("Sorceror"), _("Enough! Prepare to die..."), 180);

				self->thinkTime = 180;

				self->mental = 1;

				temp = getFreeEntity();

				if (temp == NULL)
				{
					showErrorAndExit("No free slots to add a the staff");
				}

				loadProperties("boss/sorceror_staff", temp);

				temp->action = &staffMoveAroundSorceror;

				temp->draw = &drawLoopingAnimationToMap;

				temp->type = ENEMY;

				setEntityAnimationByID(temp, 0);

				temp->x = self->x + self->w / 2 - temp->w / 2;

				temp->y = self->y + self->h / 2 - temp->h / 2;

				temp->startX = temp->x;

				temp->startY = temp->y;

				temp->head = self;

				temp->health = self->health;

				temp->mental = self->box.w;
			break;

			case 1:
				temp = getFreeEntity();

				if (temp == NULL)
				{
					showErrorAndExit("No free slots to add a the Final Attack spell");
				}

				loadProperties("boss/sorceror_final_spell", temp);

				temp->action = &finalSpellWait;

				temp->draw = &drawLoopingAnimationToMap;

				temp->type = ENEMY;

				setEntityAnimationByID(temp, 0);

				temp->x = self->x + temp->offsetX;

				temp->y = self->y + temp->offsetY;

				temp->head = self;

				temp->thinkTime = 60;

				self->thinkTime = 660;

				self->mental = 2;

				temp->health = self->health;
			break;

			default:
				self->mental = 3;

				playSoundToMap("sound/common/massive_explosion", -1, self->x, self->y, 0);

				fadeFromColour(0, 0, 200, 30);

				/* Gib the player */

				temp = self;

				self = &player;

				freeEntityList(playerGib());

				self = temp;

				self->thinkTime = 120;

				self->action = &finalAttackFinish;
			break;
		}
	}

	else if (self->thinkTime <= 420 && self->mental == 2)
	{
		self->startX = 2;
	}
}

static void finalSpellWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental++;

		if (self->mental >= 9)
		{
			self->mental = 9;
		}

		setEntityAnimationByID(self, self->mental);

		self->thinkTime = 60;
	}

	if (self->head->mental == 3 || self->health != self->head->health)
	{
		self->inUse = FALSE;
	}

	else
	{
		e = addPixelDecoration(self->x, self->y);

		if (e != NULL)
		{
			e->x = self->x + (prand() % 32) * (prand() % 2 == 0 ? -1 : 1) + self->w / 2;
			e->y = self->y + (prand() % 32) * (prand() % 2 == 0 ? -1 : 1) + self->h / 2;

			e->startX = e->x;
			e->startY = e->y;

			e->endX = self->x + self->w / 2;
			e->endY = self->y + self->h / 2;

			e->thinkTime = 15;

			e->health = 0;

			e->maxHealth = 255;

			e->mental = 0;

			calculatePath(e->startX, e->startY, e->endX, e->endY, &e->dirX, &e->dirY);
		}
	}

	self->x = self->head->x + self->offsetX;

	self->y = self->head->y + self->offsetY;
}

static void finalAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &teleportAway;
	}
}

static void staffMoveAroundSorceror()
{
	int previousX;

	self->endX += self->speed;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	previousX = self->x;

	self->x = self->startX + cos(DEG_TO_RAD(self->endX)) * self->mental;

	self->layer = previousX < self->x ? BACKGROUND_LAYER : FOREGROUND_LAYER;

	if ((self->head->flags & NO_DRAW) || self->health != self->head->health)
	{
		self->inUse = FALSE;
	}
}

static void destroyFloorInit()
{
	int rand;
	Target *t;

	rand = prand() % 2;

	t = getTargetByName(rand == 0 ? "SORCEROR_LEFT_TARGET" : "SORCEROR_RIGHT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->x = t->x;

	self->face = rand == 0 ? RIGHT : LEFT;

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->flags |= NO_DRAW;

	self->thinkTime = 60;

	self->action = &destroyFloor;

	self->mental = 0;
}

static void destroyFloor()
{
	EntityList *list;
	EntityList *l;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			self->y = getMapFloor(self->x + self->w / 2, self->y) - self->h;

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			self->flags &= ~NO_DRAW;

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->thinkTime = 60;

			self->mental = 1;
		}

		else if (self->mental == 1)
		{
			setEntityAnimation(self, "ATTACK_1");

			self->mental = 2;

			self->thinkTime = 600;

			list = getEntitiesByObjectiveName("SORCEROR_FLOOR");

			for (l=list->next;l!=NULL;l=l->next)
			{
				e = l->entity;

				e->active = TRUE;
			}

			freeEntityList(list);
		}

		else if (self->mental == 2)
		{
			setEntityAnimation(self, "STAND");

			list = getEntitiesByObjectiveName("SORCEROR_FLOOR");

			for (l=list->next;l!=NULL;l=l->next)
			{
				e = l->entity;

				e->active = FALSE;
			}

			freeEntityList(list);

			self->thinkTime = 30;

			self->action = &teleportAway;
		}
	}

	if (player.y >= getMapMaxY())
	{
		self->thinkTime = 5;

		self->mental = 2;
	}

	checkToMap(self);
}

static void riftAttackInit()
{
	int rand;
	Target *t;

	rand = prand() % 2;

	t = getTargetByName(rand == 0 ? "SORCEROR_LEFT_TARGET" : "SORCEROR_RIGHT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->x = t->x;

	self->face = rand == 0 ? RIGHT : LEFT;

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->flags |= NO_DRAW;

	self->thinkTime = 30;

	self->action = &riftAttack;
}

static void riftAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			self->y = getMapFloor(self->x + self->w / 2, self->y) - self->h;

			self->flags &= ~NO_DRAW;

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->mental = 1;

			self->thinkTime = 60;
		}

		else
		{
			setEntityAnimation(self, "ATTACK_1");

			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add an Energy Rift");
			}

			loadProperties("enemy/energy_rift", e);

			e->damage = 1;

			e->action = &riftMove;

			e->touch = &entityTouch;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;

			setEntityAnimation(e, "STAND");

			if (self->face == LEFT)
			{
				e->x = self->x - e->w;
			}

			else
			{
				e->x = self->x + self->w;
			}

			e->thinkTime = 15;

			e->y = self->y;

			e->dirX = self->face == LEFT ? -e->speed : e->speed;

			e->head = self;

			e->targetX = getMapStartX() + SCREEN_WIDTH / 2 - e->w / 2;
			e->targetY = e->y;

			e->health = 0;

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			self->mental = -1;

			self->action = &riftAttackWait;

			self->thinkTime = 60;
		}
	}

	checkToMap(self);
}

static void riftMove()
{
	self->dirX *= 0.975;

	if (fabs(self->dirX) <= 0.5)
	{
		self->dirX = 0;

		if (self->health < 2)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->health++;

				setEntityAnimation(self, self->health == 1 ? "WALK" : "JUMP");

				self->thinkTime = 15;
			}
		}

		else
		{
			self->health = playSoundToMap("sound/item/rift", -1, self->x, self->y, -1);

			self->thinkTime = 300;

			self->action = &riftWait;
		}
	}

	checkToMap(self);
}

static void riftAttackWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "STAND");
		}
	}

	if (self->mental == 0)
	{
		self->action = &teleportAway;
	}

	checkToMap(self);
}

static void riftWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		stopSound(self->health);

		self->action = &riftClose;

		self->thinkTime = 20;
	}

	else
	{
		setCustomAction(&player, &attract, 5, 0, (player.x < (self->x + self->w / 2) ? 2 : -2));

		if (prand() % 3 == 0)
		{
			addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
		}
	}
}

static void riftClose()
{
	self->thinkTime--;

	setEntityAnimation(self, self->thinkTime > 10 ? "WALK" : "STAND");

	if (self->thinkTime <= 0)
	{
		self->head->mental = 0;

		self->inUse = FALSE;
	}
}

static void addRiftEnergy(int x, int y)
{
	Entity *e;

	e = addBasicDecoration(x, y, "decoration/rift_energy");

	e->x += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);
	e->y += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);

	x -= e->w / 2;
	y -= e->h / 2;

	e->targetX = x;
	e->targetY = y;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->dirX *= 8;
	e->dirY *= 8;

	e->action = &energyMoveToRift;
}

static void energyMoveToRift()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (atTarget())
	{
		self->inUse = FALSE;
	}
}

static void disintegrationAttackInit()
{
	int rand;
	Target *t;

	rand = prand() % 2;

	t = getTargetByName(rand == 0 ? "SORCEROR_LEFT_TARGET" : "SORCEROR_RIGHT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->x = t->x;

	self->face = rand == 0 ? RIGHT : LEFT;

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->flags |= NO_DRAW;

	self->thinkTime = 30;

	self->mental = 0;

	self->action = &disintegrationAttack;
}

static void disintegrationAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			self->y = getMapFloor(self->x + self->w / 2, self->y) - self->h;

			self->flags &= ~NO_DRAW;

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->mental = 1;

			self->thinkTime = 30;
		}

		else
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Disintegration Spell");
			}

			setEntityAnimation(self, "ATTACK_2");

			loadProperties("boss/sorceror_disintegration_spell", e);

			setEntityAnimation(e, "STAND");

			e->face = self->face;

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - e->offsetX;
			}

			else
			{
				e->x = self->x + e->offsetX;
			}

			e->y = self->y + e->offsetY;

			e->action = &disintegrationSpellInit;

			e->thinkTime = 120;

			e->startX = e->x;
			e->startY = e->y;

			e->head = self;

			e->endX = player.x + player.w / 2;
			e->endY = player.y + player.h / 2;

			e->draw = &drawLoopingAnimationToMap;

			self->mental = 1;

			self->action = &disintegrationAttackWait;

			self->thinkTime = 30;
		}
	}
}

static void disintegrationAttackWait()
{
	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->startY = -1;

			self->action = &teleportAway;
		}
	}

	checkToMap(self);
}

static void disintegrationSpellInit()
{
	Entity *e = addPixelDecoration(self->x, self->y);

	if (e != NULL)
	{
		e->x = self->x + (prand() % 32) * (prand() % 2 == 0 ? -1 : 1) + self->w / 2;
		e->y = self->y + (prand() % 32) * (prand() % 2 == 0 ? -1 : 1) + self->h / 2;

		e->startX = e->x;
		e->startY = e->y;

		e->endX = self->x;
		e->endY = self->y;

		e->thinkTime = 15;

		e->health = 230;

		e->maxHealth = 230;

		e->mental = 230;

		calculatePath(e->startX, e->startY, e->endX, e->endY, &e->dirX, &e->dirY);
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/boss/sorceror/electrocute", BOSS_CHANNEL, self->x, self->y, 0);

		self->endX = player.x + player.w / 2;
		self->endY = player.y + player.h / 2;

		self->thinkTime = 300;

		self->touch = &disintegrationTouch;

		self->action = &disintegrationSpellAttack;

		self->draw = &drawDisintegrationSpell;

		self->layer = BACKGROUND_LAYER;

		self->flags &= ~NO_DRAW;
	}
}

static void disintegrationTouch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER)
	{
		if ((other->flags & BLOCKING) && strcmpignorecase(playerShield.objectiveName, "Disintegration Shield") == 0)
		{
			if (self->mental != 3)
			{
				self->dirX = self->head->face == LEFT ? -1 : 1;

				temp = self;

				self = other;

				other->takeDamage(temp, 1);

				self = temp;

				self->mental = 3;

				self->thinkTime = 30;

				stopSound(BOSS_CHANNEL);
			}
		}

		else
		{
			if (self->mental != 2)
			{
				setPlayerLocked(TRUE);

				setPlayerLocked(FALSE);

				self->thinkTime = 180;

				other->dirX = 0;

				setCustomAction(other, &helpless, self->thinkTime, 0, 0);

				setEntityAnimation(other, "CUSTOM_1");

				self->mental = 2;

				playSoundToMap("sound/boss/sorceror/electrocute", BOSS_CHANNEL, self->x, self->y, 0);
			}
		}
	}
}

static void disintegrationSpellAttack()
{
	Entity *temp;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->mental = 0;

		self->inUse = FALSE;

		if (self->mental == 2)
		{
			/* Gib the player */

			temp = self;

			self = &player;

			freeEntityList(playerGib());

			self = temp;
		}

		stopSound(BOSS_CHANNEL);
	}

	self->endX = player.x + player.w / 2;
	self->endY = player.y + player.h / 2;

	self->x = self->endX;
	self->y = self->endY;
}

static void flameWaveDropInit()
{
	Target *t;

	setEntityAnimation(self, "ATTACK_3");

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->flags |= NO_DRAW;

	t = getTargetByName("SORCEROR_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	self->y = t->y;

	self->thinkTime = 60;

	self->action = &flameWaveDrop;
}

static void flameWaveDrop()
{
	int i;
	long onGround;

	if (self->thinkTime > 0)
	{
		self->flags |= INVULNERABLE;

		self->thinkTime--;

		self->x = player.x + player.w / 2 - self->w / 2;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags &= ~(FLY|NO_DRAW);

			self->dirY = 0;
		}
	}

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		playSoundToMap("sound/enemy/red_grub/thud", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(LIGHT, 15);

		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		self->mental = 5;

		self->thinkTime = 30;

		self->action = &flameWaveAttack;
	}
}

static void flameWaveAttack()
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
				showErrorAndExit("No free slots to add the Sorceror's Flame Wave");
			}

			loadProperties("boss/sorceror_flame_wave", e);

			setEntityAnimation(e, "GROW_INIT");

			e->face = i == 0 ? LEFT : RIGHT;

			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y + self->h - e->h;

			e->action = &flameWaveMove;

			e->draw = &drawLoopingAnimationToMap;

			e->touch = &entityTouch;

			e->animationCallback = &flameWaveAnim;
		}

		self->mental--;

		self->thinkTime = 60;

		if (self->mental <= 0)
		{
			self->action = &teleportAway;
		}
	}

	checkToMap(self);
}

static void flameWaveMove()
{
	int startX, x;
	Entity *e;

	switch (self->maxThinkTime)
	{
		case 1:
			startX = getMapStartX();

			x = self->x + (self->face == RIGHT ? -self->w : self->w);

			if (!(x <= startX || x >= startX + SCREEN_WIDTH - self->w))
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Sorceror's Flame Wave");
				}

				loadProperties("boss/sorceror_flame_wave", e);

				setEntityAnimation(e, "GROW_INIT");

				e->face = self->face;

				e->x = x;
				e->y = self->y;

				e->action = &flameWaveMove;

				e->draw = &drawLoopingAnimationToMap;

				e->touch = &entityTouch;

				e->animationCallback = &flameWaveAnim;
			}

			setEntityAnimation(self, "GROW");

			self->animationCallback = &flameWaveAnim;

			self->maxThinkTime = 2;
		break;

		case 3:
			self->inUse = FALSE;
		break;

		default:

		break;
	}
}

static void flameWaveAnim()
{
	self->maxThinkTime++;
}

static void teleportAway()
{
	int rand;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		rand = prand() % 2;

		t = getTargetByName(rand == 0 ? "SORCEROR_LEFT_TARGET" : "SORCEROR_RIGHT_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Sorceror cannot find target");
		}

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->flags |= NO_DRAW|FLY;

		self->x = t->x;
		self->y = t->y;

		self->face = rand == 0 ? RIGHT : LEFT;

		self->action = &teleportAwayFinish;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void teleportAwayFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->flags &= ~NO_DRAW;

		self->action = &attackFinished;

		self->endY = self->y;

		self->endX = 0;
	}

	checkToMap(self);
}

static void statueAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->thinkTime = 15;

	self->action = &statueAttack;

	hover();
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

		e->y = getMapFloor(self->x + self->w / 2, self->y);

		e->targetY = e->y - e->h;

		e->action = &statueRise;

		e->touch = NULL;

		e->thinkTime = 240;

		e = addEnemy("enemy/flame_statue", 0, 0);

		e->health = 5;

		e->layer = BACKGROUND_LAYER;

		e->x = player.x + player.w / 2 - e->w / 2;

		e->x += player.w * 2;

		e->mental = 1;

		e->y = getMapFloor(self->x + self->w / 2, self->y);

		e->targetY = e->y - e->h;

		e->action = &statueRise;

		e->touch = NULL;

		e->thinkTime = 240;

		self->thinkTime = 600;

		self->action = &statueRiseWait;
	}

	hover();
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

			playSoundToMap("sound/common/crumble", BOSS_CHANNEL, self->x, self->y, 0);

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

	hover();
}

static void holdPersonInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->thinkTime = 15;

	self->action = &holdPerson;

	self->mental = 3;

	hover();
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

			self->thinkTime = 180;
		}

		else
		{
			self->thinkTime = 5;
		}
	}

	hover();
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

	setCustomAction(&player, &stickToFloor, 3, 0, 0);

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
		if (self->mental == 3)
		{
			setInfoBoxMessage(0, 255, 255, 255, _("Quickly turn left and right to remove the hold person spell!"));
		}

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
		setEntityAnimation(self, "ATTACK_2");

		self->mental = 3;

		self->action = &castLightningBolt;
	}

	hover();
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

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		e->action = &lightningBolt;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= FLY|DO_NOT_PERSIST;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 60;

			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	hover();
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
		playSoundToMap("sound/enemy/thunder_cloud/lightning", -1, self->targetX, self->startY, 0);

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
	self->thinkTime = 60;

	self->action = &createShield;

	self->mental = 0;

	hover();
}

static void createShield()
{
	int i, x, y;
	Entity *shield, *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			setEntityAnimation(self, "ATTACK_1");

			shield = getFreeEntity();

			if (shield == NULL)
			{
				showErrorAndExit("No free slots to add the Sorceror's Shield");
			}

			loadProperties("boss/sorceror_shield", shield);

			shield->x = self->x;
			shield->y = self->y;

			shield->action = &shieldCreateWait;

			shield->draw = &drawLoopingAnimationToMap;

			shield->touch = &entityTouch;

			shield->takeDamage = &shieldTakeDamage;

			shield->pain = &enemyPain;

			shield->die = &shieldDie;

			shield->type = ENEMY;

			shield->head = self;

			shield->alpha = 128;

			shield->face = RIGHT;

			shield->maxHealth = self->health;

			setEntityAnimation(shield, "STAND");

			shield->x = self->x + self->w / 2 - shield->w / 2;
			shield->y = self->y + self->h / 2 - shield->h / 2;

			shield->flags |= NO_DRAW;

			self->startX = 1;

			self->head = shield;

			shield->mental = 6;

			x = shield->x;
			y = shield->y;

			for (i=0;i<6;i++)
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Sorceror's Shield Piece");
				}

				loadProperties("boss/sorceror_shield_piece", e);

				setEntityAnimationByID(e, i);

				e->x = x;
				e->y = y;

				e->startX = e->x;
				e->startY = e->y;

				e->action = &pieceMoveToShield;

				e->draw = &drawLoopingAnimationToMap;

				e->type = ENEMY;

				e->alpha = 128;

				e->face = RIGHT;

				e->head = shield;

				if (x == shield->x)
				{
					x += e->w;
				}

				else
				{
					x = shield->x;

					y += e->h;
				}

				switch (i)
				{
					case 0:
						e->dirX = -12;
						e->dirY = -6;
					break;

					case 1:
						e->dirX = 12;
						e->dirY = -6;
					break;

					case 2:
						e->dirX = -12;
						e->dirY = 0;
					break;

					case 3:
						e->dirX = 12;
						e->dirY = 0;
					break;

					case 4:
						e->dirX = -12;
						e->dirY = 6;
					break;

					default:
						e->dirX = 12;
						e->dirY = 6;
					break;
				}

				e->x += e->dirX * 60;
				e->y += e->dirY * 60;

				e->dirX *= -1;
				e->dirY *= -1;
			}

			self->mental = 1;
		}

		else if (self->mental == 2)
		{
			self->thinkTime = 30;

			self->action = &createShieldFinish;
		}
	}
}

static void pieceMoveToShield()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (fabs(self->x - self->startX) <= fabs(self->dirX) && fabs(self->y - self->startY) <= fabs(self->dirY))
	{
		self->head->mental--;

		self->inUse = FALSE;
	}
}

static void shieldCreateWait()
{
	if (self->mental == 0)
	{
		self->action = &shieldWait;

		self->flags &= ~NO_DRAW;

		self->alpha = 128;

		self->head->mental = 2;
	}
}

static void createShieldFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void shieldWait()
{
	int i;
	float radians;
	Entity *e;

	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	self->thinkTime += 5;

	radians = DEG_TO_RAD(self->thinkTime);

	self->alpha = 128 + (64 * cos(radians));

	if (self->maxHealth != self->head->health)
	{
		self->die();

		for (i=0;i<8;i++)
		{
			e = addTemporaryItem("boss/sorceror_staff_piece", self->x, self->y, self->face, 0, 0);

			setEntityAnimationByID(e, i == 0 ? 0 : 1);

			e->x += (self->head->w - e->w) / 2;
			e->y += (self->head->h - e->h) / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i == 0 ? 0 : 1);

			e->thinkTime = 180 + (prand() % 60);
		}
	}
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

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (damage != 0)
	{
		if (self->startX == 1)
		{
			return;
		}

		if (self->flags & INVULNERABLE)
		{
			return;
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}
	}
}

static void shieldDie()
{
	int i, x, y;
	Entity *e;

	x = self->x;
	y = self->y;

	playSoundToMap("sound/boss/sorceror/shield_die", -1, self->x, self->y, 0);

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

	self->head->startY = 0;

	self->head->startX = 0;

	self->inUse = FALSE;
}

static void pieceWait()
{
	self->alpha -= 2;

	if (self->alpha <= 0)
	{
		self->inUse = FALSE;
	}

	self->x += self->dirX;
	self->y += self->dirY;
}

static void callSummonersInit()
{
	self->mental = 3;

	self->thinkTime = 60;

	self->action = &callSummoners;

	hover();
}

static void callSummoners()
{
	int r;
	Entity *e;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_1");

		self->mental--;

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		e = addEnemy("enemy/sorceror_dark_summoner", 0, 0);

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		r = prand() % 3;

		switch (r)
		{
			case 0:
				t = getTargetByName("DARK_SUMMONER_TARGET_0");
			break;

			case 1:
				t = getTargetByName("DARK_SUMMONER_TARGET_1");
			break;

			default:
				t = getTargetByName("DARK_SUMMONER_TARGET_2");
			break;
		}

		if (t == NULL)
		{
			showErrorAndExit("Sorceror cannot find target");
		}

		e->targetX = t->x;
		e->targetY = t->y;

		e->startX = e->targetX;
		e->startY = e->targetY;

		e->maxThinkTime = self->maxThinkTime;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		e->head = self;

		if (self->mental <= 0)
		{
			setEntityAnimation(self, "STAND");

			self->maxThinkTime++;

			self->action = &callSummonersWait;
		}

		self->thinkTime = 30;
	}

	hover();
}

static void callSummonersWait()
{
	if (self->startY == 3)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void plasmaAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->thinkTime = 30;

	self->mental = 5 + prand() % 6;

	self->action = &plasmaAttack;

	hover();
}

static void plasmaAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/boss/snake_boss/snake_boss_shot", BOSS_CHANNEL, self->x, self->y, 0);

		if (self->mental == 1)
		{
			e = addProjectile("boss/snake_boss_special_shot", self, 0, 0, 0, 0);

			e->takeDamage = &plasmaTakeDamage;
		}

		else
		{
			e = addProjectile("boss/snake_boss_normal_shot", self, 0, 0, 0, 0);
		}

		e->flags |= FLY|UNBLOCKABLE;

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = player.y + player.h / 2 - e->h / 2;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 12;
		e->dirY *= 12;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 120;

			self->action = &plasmaAttackFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	hover();
}

static void plasmaAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void plasmaTakeDamage(Entity *other, int damage)
{
	Entity *sorceror;

	sorceror = self->parent;

	self->parent = &playerWeapon;

	self->targetX = sorceror->x + sorceror->w / 2 - self->w / 2;
	self->targetY = sorceror->y + sorceror->h / 2 - self->h / 2;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 12;
	self->dirY *= 12;

	self->thinkTime = 300;

	self->damage = 50;

	self->reactToBlock = &bounceOffShield;

	self->takeDamage = NULL;
}

static void continuePoint()
{
	playBossMusic("SORCEROR_MUSIC");

	self->action = &init;
}

static void hover()
{
	self->endX++;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->y = self->endY + sin(DEG_TO_RAD(self->endX)) * 8;
}

static void die()
{
	int i;
	long onGround;

	setEntityAnimation(self, "DIE");

	self->action = &die;

	self->damage = 0;

	self->flags &= ~FLY;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		playSoundToMap("sound/enemy/red_grub/thud", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(LIGHT, 15);

		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		self->die = &entityDieNoDrop;

		self->action = &dieWait;

		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		fadeBossMusic();
	}
}

static void dieWait()
{
	checkToMap(self);
}
