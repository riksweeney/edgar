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
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/gib.h"
#include "../hud.h"
#include "../inventory.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"

extern Entity *self, player;
extern Game game;

static void initialise(void);
static void takeDamage(Entity *, int);
static void breatheFireInit(void);
static void breatheIn(void);
static void breatheFire(void);
static void flameWait(void);
static int flameDraw(void);
static void flameTouch(Entity *);
static void breatheFireFinish(void);
static void vineAttackInit(void);
static void vineAttack(void);
static void vineAttackFinish(void);
static void riftAttackInit(void);
static void riftAttack(void);
static void riftAttackWait(void);
static void riftOpen(void);
static void riftWait(void);
static void riftClose(void);
static void addRiftEnergy(int, int);
static void energyMoveToRift(void);
static void stalactiteAttackInit(void);
static void stalactiteAttack(void);
static void stalactiteFall(void);
static void stalagmiteAttackInit(void);
static void stalagmiteAttack(void);
static void stalagmiteRise(void);
static void stalagmiteWait(void);
static void stalagmiteTakeDamage(Entity *, int);
static void eatAttackInit(void);
static void eatAttack(void);
static void eatDone(void);
static void eatTouch(Entity *);
static void eatAttackWait(void);
static void eatAttackFinish(void);
static void stompAttackInit(void);
static void stompAttackReady(void);
static void stompAttack(void);
static void stompAttackFinish(void);
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
static void dieWait(void);

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
	e->takeDamage = &takeDamage;

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
		if (player.health > 0)
		{
			if (self->target != NULL && self->target->mental == 1) /* Confused */
			{
				rand = prand() % 9;
			}

			else if (self->target != NULL && self->target->mental == 2) /* Blinded */
			{
				rand = prand() % 7;
			}

			else
			{
				rand = prand() % 11;
			}

			switch (rand)
			{
				case 0:
					self->action = &breatheFireInit;
				break;

				case 1:
					self->action = &stalactiteAttackInit;
				break;

				case 2:
					self->action = &eatAttackInit;
				break;

				case 3:
					self->action = &stalagmiteAttackInit;
				break;

				case 4:
					self->action = &holdPersonInit;
				break;

				case 5:
					self->action = &vineAttackInit;
				break;

				case 6:
					self->action = &riftAttackInit;
				break;

				case 7:
					self->action = &spearAttackInit;
				break;

				case 8:
					self->action = &stompAttackInit;
				break;

				case 9:
					self->action = &confuseAttackInit;
				break;

				default:
					self->action = &blindAttackInit;
				break;
			}

			if (game.cheating == TRUE)
			{
				self->action = &breatheFireInit;
			}
		}
	}

	checkToMap(self);
}

static void riftAttackInit()
{
	createAutoDialogBox(_("Chaos"), _("Rift"), 120);

	self->thinkTime = 120;

	self->action = &riftAttack;
}

static void riftAttack()
{
	int i;
	Entity *e;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 2;

		for (i=0;i<self->mental;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add an Energy Rift");
			}

			loadProperties("enemy/energy_rift", e);

			e->damage = 1;

			e->action = &riftOpen;

			e->touch = &entityTouch;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;

			setEntityAnimation(e, "STAND");

			t = getTargetByName(i == 0 ? "CHAOS_TARGET_LEFT" : "CHAOS_TARGET_RIGHT");

			if (t == NULL)
			{
				showErrorAndExit("Chaos cannot find target");
			}

			e->thinkTime = 15;

			e->y = self->y;

			e->head = self;

			e->health = 0;
		}

		self->action = &riftAttackWait;

		self->thinkTime = 15;
	}

	checkToMap(self);
}

static void riftAttackWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void riftOpen()
{
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
		self->health = playSoundToMap("sound/item/rift.ogg", -1, self->x, self->y, -1);

		self->thinkTime = 300;

		self->action = &riftWait;
	}

	checkToMap(self);
}

static void riftWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		stopSound(self->health);

		self->action = &riftClose;

		self->thinkTime = 20;
	}

	else
	{
		if (collision(self->x - 128, self->y - 128, 256, 256, player.x, player.y, player.w, player.h) == 1)
		{
			setCustomAction(&player, &attract, 5, 0, (player.x < (self->x + self->w / 2) ? 2 : -2));
		}

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
		self->head->mental--;

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

static void vineAttackInit()
{
	self->thinkTime = 120;

	createAutoDialogBox(_("Chaos"), _("Tendrils"), 120);

	self->action = &vineAttack;

	checkToMap(self);
}

static void vineAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		activateEntitiesWithObjectiveName("CHAOS_VINES", TRUE);

		self->thinkTime = 120;

		self->action = &vineAttackFinish;
	}

	checkToMap(self);
}

static void vineAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void eatAttackInit()
{
	self->thinkTime = 60;

	self->action = &eatAttack;

	checkToMap(self);
}

static void eatAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "EAT");

		self->animationCallback = &eatDone;

		self->touch = &eatTouch;
	}
}

static void eatTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		self->flags |= GRABBING;

		other->flags |= NO_DRAW;

		self->thinkTime = 600;

		self->mental = self->health - 80;
	}
}

static void eatDone()
{
	setEntityAnimation(self, "STAND");

	self->action = &eatAttackWait;
}

static void eatAttackWait()
{
	if (self->flags & GRABBING)
	{
		player.x = self->offsetX;
		player.y = self->offsetY;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			player.health--;

			self->thinkTime = 60;

			if (player.health <= 0)
			{
				removeInventoryItemByObjectiveName("Amulet of Resurrection");

				self->action = &attackFinished;
			}
		}

		if (self->health <= self->mental)
		{
			self->action = &eatAttackFinish;
		}
	}

	else
	{
		self->action = &attackFinished;
	}
}

static void eatAttackFinish()
{
	setEntityAnimation(self, "OPEN_MOUTH");

	setCustomAction(&player, &invulnerable, 60, 0, 0);

	setPlayerStunned(30);

	player.x -= player.dirX;
	player.y -= player.dirY;

	player.dirX = (10 + prand() % 3) * (self->face == LEFT ? -1 : 1);
	player.dirY = -3;

	self->touch = &entityTouch;

	self->flags &= ~GRABBING;

	self->action = &attackFinished;
}

static void stompAttackInit()
{
	setEntityAnimation(self, "STOMP_RAISE");

	self->animationCallback = &stompAttackReady;

	self->thinkTime = 30;

	self->action = &stompAttack;

	self->mental = 1;

	checkToMap(self);
}

static void stompAttackReady()
{
	setEntityAnimation(self, "STOMP_READY");

	self->mental = 0;
}

static void stompAttack()
{
	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "STOMP");

			shakeScreen(STRONG, 120);

			activateEntitiesValueWithObjectiveName("CHAOS_ROCK_DROPPER", 5);

			if (player.flags & ON_GROUND)
			{
				setPlayerStunned(120);
			}

			self->thinkTime = 60;

			self->action = &stompAttackFinish;
		}
	}

	checkToMap(self);
}

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void stalagmiteAttackInit()
{
	createAutoDialogBox(_("Chaos"), _("Stalagmite"), 120);

	self->mental = 2 + prand() % 4;

	self->action = &stalagmiteAttack;

	self->thinkTime = 0;

	checkToMap(self);
}

static void stalagmiteAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a stalagmite");
		}

		loadProperties("enemy/stalagmite", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapFloor(self->x, self->y);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		setEntityAnimation(e, "STAND");

		e->action = &stalagmiteRise;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &entityTouch;

		e->takeDamage = &stalagmiteTakeDamage;

		e->die = &entityDieNoDrop;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= DO_NOT_PERSIST;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 60;

			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 180;
		}
	}

	checkToMap(self);
}

static void stalagmiteRise()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->y > self->startY)
		{
			self->y -= self->speed * 2;
		}

		else
		{
			playSoundToMap("sound/common/crumble.ogg", -1, self->x, self->y, 0);

			shakeScreen(MEDIUM, 15);

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

			self->y = self->startY;

			self->health = 15;

			self->thinkTime = 120;

			self->action = &stalagmiteWait;
		}
	}
}

static void stalagmiteWait()
{
	checkToMap(self);
}

static void stalagmiteTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (strcmpignorecase(self->requires, other->name) == 0)
	{
		self->health -= damage;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		if (self->health <= 0)
		{
			self->damage = 0;

			self->die();
		}
	}

	else
	{
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (other->type != PROJECTILE && prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		damage = 0;
	}
}

static void stalactiteAttackInit()
{
	createAutoDialogBox(_("Chaos"), _("Stalactite"), 120);

	self->mental = 2 + prand() % 4;

	self->action = &stalactiteAttack;

	self->thinkTime = 0;

	checkToMap(self);
}

static void stalactiteAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a stalactite");
		}

		loadProperties("item/stalactite", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapCeiling(self->x, self->y);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		setEntityAnimation(e, "STAND");

		e->action = &stalactiteFall;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= DO_NOT_PERSIST;

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

	checkToMap(self);
}

static void stalactiteFall()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->inUse = FALSE;
	}
}

static void breatheFireInit()
{
	setEntityAnimation(self, "BREATHE_IN");
	
	self->thinkTime = 240;

	/*playSoundToMap("sound/boss/chaos/breathe_in.ogg", BOSS_CHANNEL, self->x, self->y, 0);*/

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

	e = addSmoke(0, 0, self->y + self->h, "decoration/dust");

	if (e != NULL)
	{
		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		calculatePath(e->x, e->y, self->x + self->w / 2, e->y, &e->dirX, &e->dirY);

		e->dirX *= 6;
		e->dirY *= 6;
	}

	setCustomAction(&player, &attract, 2, 0, (player.x < (self->x + self->w / 2) ? (player.speed - 0.5) : -(player.speed - 0.5)));

	checkToMap(self);
}

static void breatheFire()
{
	Entity *e;

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

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Chaos's Fire");
		}

		loadProperties("boss/chaos_flame", e);

		setEntityAnimation(e, "HEAD");

		e->face = self->face;

		e->action = &flameWait;

		e->draw = &flameDraw;

		e->thinkTime = 180;

		e->mental = 1;

		e->health = 1200;

		e->head = self;

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - e->offsetX;
		}

		else
		{
			e->x = self->x + e->offsetX;
		}

		e->y = self->y + e->offsetY;

		e->dirX = 14;

		e->startX = e->x;

		e->endX = e->face == LEFT ? getMapStartX() - SCREEN_WIDTH : getMapStartX() + SCREEN_WIDTH;

		playSoundToMap("sound/boss/chaos/flame.ogg", BOSS_CHANNEL, self->x, self->y, -1);

		self->thinkTime = 60;

		self->maxThinkTime = 1;

		self->action = &breatheFireFinish;
	}

	checkToMap(self);
}

static int flameDraw()
{
	int frame;
	float timer;

	self->x = self->startX;

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	if (self->face == LEFT)
	{
		self->x -= self->w;

		setEntityAnimation(self, "BODY");

		self->currentFrame = frame;
		self->frameTimer = timer;

		while (self->x >= self->endX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	else
	{
		self->x += self->w;

		setEntityAnimation(self, "BODY");

		self->currentFrame = frame;
		self->frameTimer = timer;

		while (self->x <= self->endX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	setEntityAnimation(self, "HEAD");

	self->currentFrame = frame;
	self->frameTimer = timer;

	return TRUE;
}

static void flameWait()
{
	int startX;
	Entity *e;

	self->health--;

	if (self->health % 10 == 0)
	{
		startX = self->face == LEFT ? self->endX : self->startX;

		if (collision(player.x, player.y, player.w, player.h, startX, self->y, abs(self->startX - self->endX), self->h) == 1)
		{
			e = addProjectile("enemy/fireball", self->head, 0, 0, (self->face == LEFT ? -self->dirX : self->dirX), 0);

			e->touch = &flameTouch;

			e->element = DRAGON_FIRE;

			e->damage = 100;

			e->x = player.x + player.w / 2 - e->w / 2;
			e->y = player.y + player.h / 2 - e->h / 2;

			e->flags |= FLY|NO_DRAW;
		}
	}

	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}
}

static void flameTouch(Entity *other)
{
	int playerHealth;
	Entity *temp;

	if (other->type == PLAYER)
	{
		playerHealth = other->health;

		if (game.cheating == TRUE)
		{
			game.infiniteEnergy = FALSE;

			self->flags |= UNBLOCKABLE;
		}

		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;

		if (other->health != playerHealth)
		{
			other->flags |= NO_DRAW;

			removeInventoryItemByObjectiveName("Amulet of Resurrection");

			other->die();
		}
	}
}

static void breatheFireFinish()
{
	if (self->maxThinkTime <= 0)
	{
		if (player.health <= 0)
		{
			createAutoDialogBox(_("Chaos"), _("Pathetic"), 300);
		}

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

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		/* Take minimal damage from bombs */

		if (other->type == EXPLOSION || strcmpignorecase(other->name, "weapon/spike") == 0)
		{
			damage = 1;
		}

		self->health -= damage;

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
			self->damage = 0;

			if (other->type == WEAPON || other->type == PROJECTILE)
			{
				increaseKillCount();
			}

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

static void die()
{
	setEntityAnimation(self, "DIE_1");

	if (self->flags & GRABBING)
	{
		self->touch = &entityTouch;

		setCustomAction(&player, &invulnerable, 60, 0, 0);

		setPlayerStunned(30);

		player.x -= player.dirX;
		player.y -= player.dirY;

		player.dirX = (10 + prand() % 3) * (self->face == LEFT ? -1 : 1);
		player.dirY = -3;
	}

	self->mental = 0;

	self->thinkTime = 180;

	self->damage = 0;

	self->takeDamage = NULL;

	self->action = &dieWait;

	checkToMap(self);
}

static void dieWait()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			shakeScreen(MEDIUM, 60);

			playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

			for (i=0;i<30;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}

			self->mental = 1;

			setEntityAnimation(self, "DIE_2");

			self->thinkTime = 60;
		}

		else
		{
			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);

			clearContinuePoint();

			freeBossHealthBar();
		}
	}

	checkToMap(self);
}

static void creditsAction()
{

}
