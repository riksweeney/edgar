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
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
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
static void touch(Entity *);
static void takeDamage(Entity *, int);
static void breatheFireInit(void);
static void breatheIn(void);
static void breatheFire(void);
static void flameWait(void);
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
static void spinnerAttackInit(void);
static void spinnerAttack(void);
static void spinnerAttackFinish(void);
static void spinnerMove(void);
static void rotateAroundTarget(void);
static void spinnerPartDie(void);
static void stalagmiteAttackInit(void);
static void stalagmiteAttack(void);
static void stalagmiteAttackFinish(void);
static void stalagmiteRise(void);
static void stalagmiteWait(void);
static void stalagmiteTakeDamage(Entity *, int);
static void stalagmiteDie(void);
static void eatAttackInit(void);
static void eatAttack(void);
static void eatTouch(Entity *);
static void eatAttackWait(void);
static void eatAttackFinish(void);
static void stompAttackInit(void);
static void stompAttackRise(void);
static void stompAttackDrop(void);
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
static void continuePoint(void);
static void addLegendarySword(void);
static void swordWait(void);
static void swordTouch(Entity *);
static void creditsDone(void);

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

	e->action = &addLegendarySword;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
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
		setContinuePoint(FALSE, self->name, &continuePoint);

		initBossHealthBar();

		self->thinkTime = 60;

		self->maxThinkTime = 0;

		self->action = &entityWait;
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
			if (self->target != NULL && self->target->inUse == TRUE && self->target->mental == 1) /* Confused */
			{
				rand = prand() % 8;
			}

			else if (self->target != NULL && self->target->inUse == TRUE && self->target->mental == 2) /* Blinded */
			{
				rand = prand() % 4;
			}

			else
			{
				rand = prand() % 10;
			}

			switch (rand)
			{
				case 0:
					self->action = &eatAttackInit;
				break;

				case 1:
					self->action = &stalagmiteAttackInit;
				break;

				case 2:
					self->action = &holdPersonInit;
				break;

				case 3:
					self->action = &vineAttackInit;
				break;

				case 4:
					self->action = &riftAttackInit;
				break;

				case 5:
					self->action = &spearAttackInit;
				break;

				case 6:
					self->action = &spinnerAttackInit;
				break;

				case 7:
					self->action = &stompAttackInit;
				break;

				case 8:
					self->action = &confuseAttackInit;
				break;

				default:
					self->action = &blindAttackInit;
				break;
			}

			if (game.cheating == TRUE || self->maxThinkTime <= 0 || player.weight == 0)
			{
				self->action = &breatheFireInit;
			}
		}
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void riftAttackInit()
{
	createAutoDialogBox(_("Chaos"), _("Rift"), 90);

	self->thinkTime = 60;

	self->action = &riftAttack;

	self->maxThinkTime--;

	checkToMap(self);
}

static void riftAttack()
{
	int i, width;
	Entity *e;
	Target *t1, *t2;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 2;

		t1 = getTargetByName("CHAOS_TARGET_LEFT");

		t2 = getTargetByName("CHAOS_TARGET_RIGHT");

		if (t1 == NULL || t2 == NULL)
		{
			showErrorAndExit("Chaos cannot find target");
		}

		width = (t2->x - t1->x) / 2;

		for (i=0;i<self->mental;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add an Energy Rift");
			}

			loadProperties("enemy/energy_rift", e);

			e->damage = 2;

			e->action = &riftOpen;

			e->touch = &entityTouch;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;

			setEntityAnimation(e, "STAND");

			e->x = (i == 0 ? t1->x : t2->x) - e->w / 2;
			e->y = (i == 0 ? t1->y : t2->y);

			e->speed = width;

			e->thinkTime = 15;

			e->head = self;

			e->health = 0;
		}

		self->action = &riftAttackWait;

		self->thinkTime = 15;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void riftAttackWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}

	self->maxThinkTime--;

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
		self->health = playSoundToMap("sound/item/rift", -1, self->x, self->y, -1);

		self->thinkTime = 300;

		self->action = &riftWait;
	}

	checkToMap(self);
}

static void riftWait()
{
	int x, y;

	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		stopSound(self->health);

		self->action = &riftClose;

		self->thinkTime = 20;
	}

	else
	{
		x = self->x + self->w / 2;
		y = self->y + self->h / 2;

		if (collision(x - self->speed, y - self->speed, self->speed * 2, self->speed * 2, player.x, player.y, player.w, player.h) == 1)
		{
			setCustomAction(&player, &attract, 5, 0, (player.x < (self->x + self->w / 2) ? player.speed - 0.25 : -(player.speed - 0.25)));
		}

		if (prand() % 3 == 0)
		{
			addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
		}
	}

	checkToMap(self);
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

	checkToMap(self);
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

	createAutoDialogBox(_("Chaos"), _("Tendrils"), 90);

	self->action = &vineAttack;

	self->maxThinkTime--;

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

	self->maxThinkTime--;

	checkToMap(self);
}

static void vineAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void eatAttackInit()
{
	Target *t = getTargetByName("CHAOS_EAT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Chaos cannot find target");
	}

	self->thinkTime = 60;

	self->action = &eatAttack;

	self->targetX = t->x;

	self->maxThinkTime--;

	checkToMap(self);
}

static void eatAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->x <= self->targetX)
		{
			self->x = self->targetX;

			setEntityAnimation(self, "MOUTH_CLOSE");

			if (self->flags & GRABBING)
			{
				self->mental = self->health - 80;

				self->thinkTime = 600;

				self->action = &eatAttackWait;
			}

			else
			{
				self->thinkTime = 60;

				self->touch = &touch;

				self->action = &eatAttackFinish;
			}

			self->targetX = self->startX;

			self->dirX = 0;
		}

		else
		{
			setEntityAnimation(self, "MOUTH_OPEN");

			self->dirX = -12;

			self->touch = &eatTouch;

			if (self->flags & GRABBING)
			{
				if (self->face == LEFT)
				{
					player.x = self->x + self->w - player.w - self->offsetX;
				}

				else
				{
					player.x = self->x + self->offsetX;
				}

				player.y = self->y + self->offsetY;
			}
		}
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void eatTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		if (!(self->flags & GRABBING) && player.weight != 0)
		{
			self->flags |= GRABBING;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void eatAttackWait()
{
	Entity *temp;

	if (self->face == LEFT)
	{
		player.x = self->x + self->w - player.w - self->offsetX;
	}

	else
	{
		player.x = self->x + self->offsetX;
	}

	player.y = self->y + self->offsetY;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		temp = self;

		self = &player;

		self->takeDamage(temp, 3);

		self = temp;

		self->thinkTime = 60;

		if (player.health <= 0)
		{
			removeInventoryItemByObjectiveName("Amulet of Resurrection");

			self->action = &attackFinished;
		}
	}

	if (self->health <= self->mental)
	{
		setEntityAnimation(self, "SPIT_OUT");

		if (self->face == LEFT)
		{
			player.x = self->x + self->w - player.w - self->offsetX;
		}

		else
		{
			player.x = self->x + self->offsetX;
		}

		player.y = self->y + self->offsetY;

		setCustomAction(&player, &invulnerable, 60, 0, 0);

		setPlayerStunned(30);

		player.x -= player.dirX;
		player.y -= player.dirY;

		player.dirX = (10 + prand() % 3) * (self->face == LEFT ? -1 : 1);
		player.dirY = -3;

		self->touch = &touch;

		self->flags &= ~GRABBING;

		self->thinkTime = 60;

		self->action = &eatAttackFinish;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void eatAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->x >= self->targetX)
		{
			setEntityAnimation(self, "STAND");

			self->x = self->startX;

			self->action = &attackFinished;

			self->dirX = 0;
		}

		else
		{
			self->dirX = 12;
		}
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void stompAttackInit()
{
	int i;
	Entity *e;

	setEntityAnimation(self, "STOMP_READY");

	self->weight = 0.5;

	self->dirY = -8;

	self->action = &stompAttackRise;

	for (i=0;i<30;i++)
	{
		e = addSmoke(0, 0, "decoration/dust");

		if (e != NULL)
		{
			e->x = self->x + prand() % self->w;
			e->y = self->y + self->h;

			e->dirX = (10 + prand() % 30);

			e->dirX /= 10;

			e->y -= prand() % e->h;

			if (e->x < self->x + self->w / 2)
			{
				e->dirX *= -1;
			}
		}
	}

	checkToMap(self);
}

static void stompAttackRise()
{
	if (self->dirY >= 0)
	{
		self->dirY = 0;

		self->flags |= FLY;

		self->thinkTime = 60;

		self->weight = 1;

		self->action = &stompAttackDrop;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void stompAttackDrop()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "STAND");

		playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

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

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void stalagmiteAttackInit()
{
	createAutoDialogBox(_("Chaos"), _("Stalagmite"), 90);

	self->mental = 1 + prand() % 3;

	self->action = &stalagmiteAttack;

	self->thinkTime = 60;

	self->maxThinkTime--;

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

		loadProperties("item/stalagmite", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapFloor(self->x, self->y);

		e->startY = e->targetY - e->h;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		setEntityAnimation(e, "STAND");

		e->action = &stalagmiteRise;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &entityTouch;

		e->damage = 2;

		e->takeDamage = &stalagmiteTakeDamage;

		e->die = &stalagmiteDie;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= DO_NOT_PERSIST;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 60;

			self->action = &stalagmiteAttackFinish;
		}

		else
		{
			self->thinkTime = 180;
		}
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void stalagmiteAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	self->maxThinkTime--;

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
			self->y -= 12;
		}

		else
		{
			playSoundToMap("sound/common/crumble", -1, self->x, self->y, 0);

			shakeScreen(MEDIUM, 15);

			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = -3;
			e->dirY = -8;

			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = 1;
			e->dirY = -8;

			self->y = self->startY;

			self->action = &stalagmiteWait;
		}
	}
}

static void stalagmiteWait()
{
	if (self->head->health <= 0)
	{
		self->die();
	}

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

		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

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

static void stalagmiteDie()
{
	Entity *e;

	playSoundToMap("sound/common/crumble", -1, self->x, self->y, 0);

	e = addSmallRock(self->x, self->y, "common/small_rock");

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirX = -3;
	e->dirY = -8;

	e = addSmallRock(self->x, self->y, "common/small_rock");

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirX = 1;
	e->dirY = -8;

	self->inUse = FALSE;
}

static void spinnerAttackInit()
{
	createAutoDialogBox(_("Chaos"), _("Orbs"), 90);

	self->mental = 3 + prand() % 4;

	self->action = &spinnerAttack;

	self->thinkTime = 60;

	self->maxThinkTime--;

	checkToMap(self);
}

static void spinnerAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a spinner");
		}

		loadProperties("boss/snake_boss_special_shot", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->targetX = self->x - e->w;
		e->targetY = self->y + self->h / 2;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		e->flags |= FLY|NO_DRAW;

		e->head = self;

		e->face = RIGHT;

		e->action = &spinnerMove;

		e->draw = &drawLoopingAnimationToMap;

		e->type = ENEMY;

		self->action = &spinnerAttackFinish;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void spinnerAttackFinish()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void spinnerMove()
{
	int i;
	Entity *e;

	if (!(self->flags & NO_DRAW))
	{
		for (i=0;i<8;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a spinner part");
			}

			loadProperties("boss/snake_boss_special_shot", e);

			setEntityAnimation(e, "STAND");

			e->x = self->x;
			e->y = self->y;

			e->startX = i * 30;

			e->mental = 0;

			e->damage = 2;

			e->health = 30;

			e->speed = 3;

			e->flags |= FLY;

			e->head = self;

			e->face = RIGHT;

			e->action = &rotateAroundTarget;

			e->touch = &entityTouch;

			e->die = &spinnerPartDie;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;
		}

		self->flags |= NO_DRAW;

		self->targetX = getMapStartX() - 128;

		self->dirX = -1;
	}

	checkToMap(self);

	if (self->x <= self->targetX && (self->flags & NO_DRAW))
	{
		self->head->mental = 0;

		self->inUse = FALSE;
	}
}

static void rotateAroundTarget()
{
	float radians;

	self->mental += 4;

	if (self->mental > 128)
	{
		self->mental = 128;
	}

	self->startX += self->speed;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	radians = DEG_TO_RAD(self->startX);

	self->x = self->head->x;
	self->y = self->head->y;

	self->x += (0 * cos(radians) - self->mental * sin(radians));
	self->y += (0 * sin(radians) + self->mental * cos(radians));

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void spinnerPartDie()
{
	self->head->health--;

	entityDieNoDrop();
}

static void breatheFireInit()
{
	setEntityAnimation(self, "BREATHE_IN");

	self->thinkTime = 150;

	playSoundToMap("sound/boss/chaos/breathe_in", BOSS_CHANNEL, self->x, self->y, 0);

	self->action = &breatheIn;

	self->creditsAction = &breatheIn;

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

		self->creditsAction = &breatheFire;
	}

	e = addSmoke(0, 0, "decoration/dust");

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

		e->targetX = e->x;
		e->targetY = e->y;

		e->x += (128 + prand() % 128) * (self->face == LEFT ? -1 : 1);
		e->y += (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 6;
		e->dirY *= 6;

		e->thinkTime = 30;
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
		setEntityAnimation(self, "BREATHE_FIRE");

		switch (prand() % 4)
		{
			case 0:
				createAutoDialogBox(_("Chaos"), _("Burn to ashes!"), 180);
			break;

			case 1:
				createAutoDialogBox(_("Chaos"), _("Feel my power!"), 180);
			break;

			case 2:
				createAutoDialogBox(_("Chaos"), _("I will destroy you!"), 180);
			break;

			default:
				createAutoDialogBox(_("Chaos"), _("Suffer!"), 180);
			break;
		}

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Chaos's Fire");
		}

		loadProperties("boss/chaos_flame", e);

		e->face = self->face;

		e->action = &flameWait;

		e->creditsAction = &flameWait;

		e->draw = &drawLoopingAnimationToMap;

		e->thinkTime = 300;

		if (game.status != IN_CREDITS)
		{
			shakeScreen(LIGHT, 300);
		}

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

		playSoundToMap("sound/boss/chaos/breathe_fire", BOSS_CHANNEL, self->x, self->y, 0);

		self->thinkTime = 30;

		self->mental = 1;

		self->action = &breatheFireFinish;

		self->creditsAction = &breatheFireFinish;
	}

	checkToMap(self);
}

static void flameWait()
{
	Entity *e;

	self->health--;

	player.flags &= ~FLY;

	if (self->health % 10 == 0)
	{
		if (collision(player.x, player.y, player.w, player.h, self->x, self->y, self->w, self->h) == 1)
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
		self->head->mental = 0;

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
			setPlayerAsh();

			other->flags |= NO_DRAW;

			removeInventoryItemByObjectiveName("Amulet of Resurrection");

			other->die();

			other->mental = 1;
		}
	}
}

static void breatheFireFinish()
{
	if (self->mental == 0)
	{
		setEntityAnimation(self, "MOUTH_OPEN");

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "STAND");

			if (player.health <= 0)
			{
				player.mental = 0;

				createAutoDialogBox(_("Chaos"), _("Pathetic"), 300);
			}

			self->action = &attackFinished;

			self->creditsAction = &creditsDone;

			self->maxThinkTime = 30 * 60;
		}
	}

	checkToMap(self);
}

static void spearAttackInit()
{
	self->thinkTime = 60;

	createAutoDialogBox(_("Chaos"), _("Impale"), 90);

	self->action = &spearAttack;

	self->maxThinkTime--;

	checkToMap(self);
}

static void spearAttack()
{
	int i, j, startX;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
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

			e->damage = 2;

			e->touch = &entityTouch;

			e->action = &spearWait;

			e->draw = &drawLoopingAnimationToMap;

			e->head = self;

			i -= e->w * 2;

			self->mental++;

			j++;
		}

		j = 1;

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

			e->damage = 2;

			e->touch = &entityTouch;

			e->action = &spearWait;

			e->draw = &drawLoopingAnimationToMap;

			e->head = self;

			i += e->w * 2;

			self->mental++;

			j++;
		}

		self->action = &spearAttackFinished;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void spearAttackFinished()
{
	if (self->mental <= 0)
	{
		self->action = &attackFinished;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void confuseAttackInit()
{
	self->thinkTime = 120;

	createAutoDialogBox(_("Chaos"), _("Confuse"), 90);

	self->action = &confuseAttack;

	self->maxThinkTime--;

	checkToMap(self);
}

static void confuseAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		fadeFromColour(0, 200, 0, 30);

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

		e->head = self;

		setPlayerConfused(e->thinkTime);

		self->action = &attackFinished;

		self->target = e;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void confuseSpellWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		self->inUse = FALSE;
	}
}

static void blindAttackInit()
{
	self->thinkTime = 120;

	createAutoDialogBox(_("Chaos"), _("Darkness"), 90);

	self->action = &blindAttack;

	self->maxThinkTime--;

	checkToMap(self);
}

static void blindAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		fadeFromColour(0, 0, 0, 30);

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

		e->head = self;

		setDarkMap(TRUE);

		self->action = &attackFinished;

		self->target = e;
	}

	self->maxThinkTime--;

	checkToMap(self);
}

static void blindSpellWait()
{
	self->thinkTime--;

	if (self->head->flags & GRABBING)
	{
		setDarkMap(FALSE);
	}

	else
	{
		setDarkMap(TRUE);
	}

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		setDarkMap(FALSE);

		self->inUse = FALSE;
	}
}

static void holdPersonInit()
{
	createAutoDialogBox(_("Chaos"), _("Hold Person"), 90);

	self->thinkTime = 15;

	self->action = &holdPerson;

	self->mental = 3;

	self->maxThinkTime--;

	checkToMap(self);
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

		e->head = self;

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

	self->maxThinkTime--;

	checkToMap(self);
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
		if (self->face != player.face)
		{
			self->face = player.face;

			self->health--;

			if (self->health <= 0 || self->head->health <= 0)
			{
				self->health = 0;

				setEntityAnimation(self, "LEFT_PIECE");

				self->layer = FOREGROUND_LAYER;

				self->dirX = -4;

				self->action = &holdPersonPieceMove;

				self->thinkTime = 10;
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

		self->thinkTime = 10;
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
		self->mental = 3 + prand() % 3;

		self->action = &castLightningBolt;
	}

	self->maxThinkTime--;

	checkToMap(self);
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

	self->maxThinkTime--;

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

			e->damage = 3;
		}

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = 1;
		e->dirY = -8;

		self->inUse = FALSE;
	}
}

static void attackFinished()
{
	self->thinkTime = 60;

	self->action = &entityWait;

	self->maxThinkTime--;

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
			playSoundToMap("sound/enemy/ground_spear/spear", -1, self->x, self->y, 0);

			self->targetY = self->startY;

			self->action = &spearRise;
		}
	}

	if (self->head->health <= 0 && self->y == self->endY)
	{
		self->inUse = FALSE;
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

static void touch(Entity *other)
{
	if (game.cheating == TRUE && other->type == PLAYER && other->health > 0)
	{
		removeInventoryItemByObjectiveName("Amulet of Resurrection");

		other->die();
	}

	else
	{
		entityTouch(other);
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	player.flags &= ~FLY;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		if (game.cheating == TRUE)
		{
			damage = 0;
		}

		if (other->element == FIRE)
		{
			self->health += damage;

			if (other->type == PROJECTILE)
			{
				other->target = self;
			}

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

			if (prand() % 5 == 0)
			{
				setInfoBoxMessage(90, 255, 255, 255, _("The damage from this weapon is being absorbed..."));
			}

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}
		}

		else
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
}

static void die()
{
	self->flags &= ~FLY;

	self->weight = 1;

	playSoundToMap("sound/boss/chaos/die", BOSS_CHANNEL, self->x, self->y, 0);

	setEntityAnimation(self, "DIE_1");

	self->animationCallback = NULL;

	activateEntitiesWithObjectiveName("CHAOS_VINES", FALSE);

	self->x = self->startX;

	if (self->flags & GRABBING)
	{
		if (self->face == LEFT)
		{
			player.x = self->x + self->w - player.w - self->offsetX;
		}

		else
		{
			player.x = self->x + self->offsetX;
		}

		player.y = self->y + self->offsetY;

		setCustomAction(&player, &invulnerable, 60, 0, 0);

		setPlayerStunned(30);

		player.x -= player.dirX;
		player.y -= player.dirY;

		player.dirX = (10 + prand() % 3) * (self->face == LEFT ? -1 : 1);
		player.dirY = -3;

		self->touch = &entityTouch;
	}

	self->mental = 0;

	self->thinkTime = 420;

	self->damage = 0;

	self->takeDamage = NULL;

	self->action = &dieWait;

	fadeOutMusic(3000);

	checkToMap(self);
}

static void dieWait()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->mental == 0)
	{
		self->x = self->startX + sin(DEG_TO_RAD(self->endX)) * 4;

		self->endX += 90;

		if (self->endX >= 360)
		{
			self->endX = 0;
		}
	}

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			self->x = self->startX;

			self->layer = MID_GROUND_LAYER;

			setEntityAnimation(self, "DIE_2");

			shakeScreen(MEDIUM, 60);

			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

			for (i=0;i<300;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}

			self->mental = 1;

			self->thinkTime = 120;
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
	if (self->active == TRUE)
	{
		self->alpha += 2;

		if (self->alpha >= 255)
		{
			self->alpha = 255;

			self->creditsAction = &breatheFireInit;
		}
	}
}

static void continuePoint()
{
	self->action = &addLegendarySword;

	playMapMusic();

	checkToMap(self);
}

static void addLegendarySword()
{
	Entity *e;

	e = addPermanentItem("weapon/legendary_sword", 0, 0);

	e->x = self->x;
	e->y = self->y;

	e->flags |= DO_NOT_PERSIST;

	e->action = &swordWait;

	e->touch = NULL;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	setEntityAnimation(e, "STICK_IN_CHAOS");

	self->action = &initialise;
}

static void swordWait()
{
	if (self->head->health <= 0)
	{
		setEntityAnimation(self, "STICK_IN_CHAOS_DIE");

		self->touch = &swordTouch;
	}

	self->face = self->head->face;

	self->layer = self->head->layer;

	if (self->head->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void creditsDone()
{
	self->inUse = FALSE;
}

static void swordTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		self->head = NULL;

		self->action = &doNothing;

		self->touch = &keyItemTouch;
	}
}
