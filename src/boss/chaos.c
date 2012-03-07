/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../custom_actions.h"
#include "../collisions.h"
#include "../dialog.h"
#include "../enemy/rock.h"
#include "../enemy/thunder_cloud.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/gib.h"
#include "../hud.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void initialise(void);
static void breatheFireInit(void);
static void breatheIn(void);
static void breatheFire(void);
static void breatheFireFinish(void);
static void spearAttackInit(void);
static void spearAttack(void);
static void spearAttackFinished(void);
static void blindAttackInit(void);
static void blindAttack(void);
static void blindSpellWait(void);
static void confuseAttackInit(void);
static void confuseAttack(void);
static void confuseSpellWait(void);
static void holdPersonInit(void);
static void holdPerson(void);
static void holdPersonSpellMove(void);
static void holdPersonBackWait(void);
static void holdPersonPieceMove(void);
static void holdPersonWait(void);
static void castLightningBolt(void);
static void lightningBolt(void);
static void attackFinished(void);
static void entityWait(void);
static void spearWait(void);
static void spearRise(void);
static void spearSink(void);
static void creditsAction(void);
static void die(void);

Entity *addChaos(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Chaos");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &entityTakeDamageNoFlinch;

	e->creditsAction = &creditsAction;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->action = &breatheFireInit;
	}

	checkToMap(self);
}

static void entityWait()
{
	int rand;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		rand = 1;
		
		if (self->target != NULL && self->target->mental == 1) /* Confused */
		{

		}

		else if (self->target != NULL && self->target->mental == 2) /* Blinded */
		{

		}

		switch (rand)
		{
			case 0:
				self->action = &breatheFireInit;
			break;

			case 1:
				self->action = &spearAttackInit;
			break;

			case 2:
				self->action = &confuseAttackInit;
			break;

			case 3:
				self->action = &blindAttackInit;
			break;

			case 4:
				self->action = &holdPersonInit;
			break;
		}

		if (game.cheating == TRUE)
		{
			self->action = &breatheFireInit;
		}
	}

	checkToMap(self);
}

static void breatheFireInit()
{
	self->thinkTime = 240;

	playSoundToMap("sound/boss/chaos/breathe_in.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->action = &breatheIn;

	checkToMap(self);
}

static void breatheIn()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;

		self->action = &breatheFire;
	}

	e = addSmoke(self->endX + (prand() % (SCREEN_WIDTH / 2)), self->y, "decoration/dust");

	if (e != NULL)
	{
		e->y += prand() % self->h;

		calculatePath(e->x, e->y, self->x + self->w / 2, self->y + self->h / 2, &e->dirX, &e->dirY);

		e->dirX *= 6;
		e->dirY *= 6;
	}

	setCustomAction(&player, &attract, 2, 0, (player.x < (self->x + self->w / 2) ? (player.speed - 0.5) : -(player.speed - 0.5)));

	checkToMap(self);
}

static void breatheFire()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		switch (prand() % 3)
		{
			case 0:
				createAutoDialogBox(_("Chaos"), _("Burn to ashes!"), 120);
			break;

			case 1:
				createAutoDialogBox(_("Chaos"), _("Feel my power!"), 120);
			break;

			default:
				createAutoDialogBox(_("Chaos"), _("Suffer!"), 120);
			break;
		}
		
		self->maxThinkTime = 1;
		
		self->action = &breatheFireFinish;
	}

	checkToMap(self);
}

static void breatheFireFinish()
{
	if (self->maxThinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void spearAttackInit()
{
	self->thinkTime = 30;

	createAutoDialogBox(_("Chaos"), _("Impale"), 120);

	self->action = &spearAttack;

	checkToMap(self);
}

static void spearAttack()
{
	int i, j, startX;
	Entity *e;

	self->mental = 0;

	startX = getMapStartX();

	j = 1;

	for (i=self->x;i>=startX;)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Ground Spear");
		}

		loadProperties("enemy/ground_spear", e);

		setEntityAnimation(e, "STAND");

		e->x = i;
		e->y = self->y + self->h;

		e->startY = e->y - e->h;

		e->endY = e->y;

		e->thinkTime = 15 * j;

		e->damage = 1;

		e->touch = &entityTouch;

		e->action = &spearWait;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		i -= e->w * 2;

		self->mental++;

		j++;
	}

	i += e->w * 4;

	for (;i<self->x;)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Ground Spear");
		}

		loadProperties("enemy/ground_spear", e);

		setEntityAnimation(e, "STAND");

		e->x = i;
		e->y = self->y + self->h;

		e->startY = e->y - e->h;

		e->endY = e->y;

		e->thinkTime = 15 * j;

		e->damage = 1;

		e->touch = &entityTouch;

		e->action = &spearWait;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		i += e->w * 2;

		self->mental++;

		j++;
	}

	self->action = &spearAttackFinished;

	checkToMap(self);
}

static void spearAttackFinished()
{
	if (self->mental <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void confuseAttackInit()
{
	self->thinkTime = 120;

	createAutoDialogBox(_("Chaos"), _("Confuse"), 120);
	
	self->action = &confuseAttack;
}

static void confuseAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Chaos confuse spell");
		}

		loadProperties("boss/sorceror_disintegration_spell", e);

		e->action = &confuseSpellWait;

		e->draw = &drawLoopingAnimationToMap;

		e->flags |= NO_DRAW|FLY;

		e->x = self->x;
		e->y = self->y;

		e->thinkTime = 600;

		e->mental = 1;

		setPlayerConfused(e->thinkTime);

		self->action = &attackFinished;

		self->target = e;
	}

	checkToMap(self);
}

static void confuseSpellWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void blindAttackInit()
{
	self->thinkTime = 120;

	createAutoDialogBox(_("Chaos"), _("Darkness"), 120);
	
	self->action = &blindAttack;
}

static void blindAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Chaos blind spell");
		}

		loadProperties("boss/sorceror_disintegration_spell", e);

		e->action = &blindSpellWait;

		e->draw = &drawLoopingAnimationToMap;

		e->flags |= NO_DRAW|FLY;

		e->x = self->x;
		e->y = self->y;

		e->thinkTime = 600;

		e->mental = 2;

		setDarkMap(TRUE);

		self->action = &attackFinished;

		self->target = e;
	}

	checkToMap(self);
}

static void blindSpellWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setDarkMap(FALSE);

		self->inUse = FALSE;
	}
}

static void holdPersonInit()
{
	createAutoDialogBox(_("Chaos"), _("Hold Person"), 120);

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
			showErrorAndExit("No free slots to add the Hold Person spell");
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
}

static void holdPersonSpellMove()
{
	Entity *e;

	if (self->target == NULL)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Hold Person spell");
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
			self->thinkTime = 60;

			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 30;
		}
	}
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

static void attackFinished()
{
	self->thinkTime = 60;

	self->action = &entityWait;

	checkToMap(self);
}

static void spearWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->y == self->startY)
		{
			self->action = &spearSink;
		}

		else
		{
			playSoundToMap("sound/enemy/ground_spear/spear.ogg", -1, self->x, self->y, 0);

			self->targetY = self->startY;

			self->action = &spearRise;
		}
	}
}

static void spearSink()
{
	if (self->y < self->endY)
	{
		self->y += self->speed * 2;
	}

	else
	{
		self->inUse = FALSE;

		self->head->mental--;
	}
}

static void spearRise()
{
	if (self->y > self->startY)
	{
		self->y -= self->speed * 2;
	}

	else
	{
		self->y = self->startY;

		self->thinkTime = 5;

		self->action = &spearWait;
	}
}

static void die()
{
	
}

static void creditsAction()
{
	
}
