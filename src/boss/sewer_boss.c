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
#include "../entity.h"
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

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
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
static void punchAttackInit(void);
static void punchAttack(void);
static void punchWait(void);
static void punchAttackFinish(void);
static void punchReturnToNormal(void);
static void slimeAttackInit(void);
static void slimeAttackMoveToTarget(void);
static void slimeAttack(void);
static void slimeAttackMouthClose(void);
static void slimeAttackFinish(void);
static void punchReactToBlock(Entity *);
static void grabTouch(Entity *);
static void grabAttackInit(void);
static void grabAttack(void);
static void grabWait(void);
static void clawTakeDamage(Entity *, int);
static void dropPlayer(void);
static void clawDie(void);
static void slimeDie(void);
static void slimeAttackBreatheIn(void);
static void addMouthOrb(void);
static void orbWait(void);
static void orbTakeDamage(Entity *, int);
static void slimePlayer(Entity *);
static void slimePillFallout(void);
static void createSlime(void);
static void fill(int, int);
static void sinkAttackInit(void);
static void sinkAttackWait(void);
static void clawSideAttackFinish(void);
static void clawSideAttackRise(void);
static void clawSideAttackWait(void);
static void sideAttackReactToBlock(Entity *);
static void clawSideAttack(void);
static void changeToWater(void);
static int draw(void);
static void armChangeToWater(void);
static int armDraw(void);
static void finalAttackRise(void);
static void die(void);
static void dieSink(void);
static void finalAttack(void);
static void finalGrabPlayer(void);
static void dragDownInit(void);
static void dragDown(void);
static void finalGrabWait(void);
static void finalClawDie(void);
static void initialAttackRise(void);
static void maggotTouch(Entity *);
static void grabMaggot(void);
static void maggotGrabWait(void);
static void struggle(void);
static void maggotGrabSink(void);
static void addSlimeDrips(void);
static void dripWait(void);
static int dripDraw(void);
static void dripChangeToWater(void);
static void creditsMove(void);

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

	e->creditsAction = &creditsMove;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		playDefaultBossMusic();

		initBossHealthBar();

		self->flags |= ATTACKING;

		setContinuePoint(FALSE, self->name, NULL);

		self->mental = 0;

		self->endX = 0;

		addMouthOrb();

		addSlimeDrips();

		addClaws();

		self->action = &doIntro;

		self->touch = &entityTouch;

		self->takeDamage = &takeDamage;

		self->dirY = -self->speed;
	}

	else if (self->mental == -60)
	{
		addClaws();

		self->mental = -65;
	}
}

static void doIntro()
{
	checkToMap(self);

	if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->dirY = 0;

		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	setEntityAnimation(self, self->element == SLIME ? "STAND" : "STAND_WATER");

	self->thinkTime = 180;

	self->action = &entityWait;
}

static void entityWait()
{
	int action, tile;

	self->thinkTime--;

	tile = mapTileAt((self->x + self->w / 2) / TILE_SIZE, (self->y + self->h - TILE_SIZE) / TILE_SIZE);

	if (tile >= SLIME_TILE_START && tile <= SLIME_TILE_END && self->element == WATER)
	{
		self->element = SLIME;
	}

	else if (tile >= WATER_TILE_START && tile <= WATER_TILE_END && self->element == SLIME)
	{
		self->element = WATER;

		self->draw = &draw;

		self->action = &changeToWater;

		self->mental = -30;

		return;
	}

	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->endX != 0)
		{
			if (self->health >= 300)
			{
				action = prand() % 5;

				switch (action)
				{
					case 0:
					case 1:
					case 2:
					case 3:
						setEntityAnimation(self, self->element == SLIME ? "ATTACK_4" : "ATTACK_4_WATER");

						self->mental = -10;

						self->action = &punchWait;
					break;

					default:
						self->dirY = self->speed;

						self->action = &changeSidesSink;
					break;
				}
			}

			else if (self->health >= 200)
			{
				action = prand() % 7;

				switch (action)
				{
					case 0:
					case 1:
					case 2:
						setEntityAnimation(self, self->element == SLIME ? "ATTACK_4" : "ATTACK_4_WATER");

						self->mental = -10;

						self->action = &punchWait;
					break;

					case 3:
					case 4:
					case 5:
						setEntityAnimation(self, self->element == SLIME ? "ATTACK_4" : "ATTACK_4_WATER");

						self->mental = -5;

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
				action = prand() % 9;

				switch (action)
				{
					case 0:
					case 1:
					case 2:
						setEntityAnimation(self, self->element == SLIME ? "ATTACK_4" : "ATTACK_4_WATER");

						self->mental = -10;

						self->action = &punchWait;
					break;

					case 3:
					case 4:
					case 5:
						setEntityAnimation(self, self->element == SLIME ? "ATTACK_4" : "ATTACK_4_WATER");

						self->mental = -5;

						self->action = &punchWait;
					break;

					case 6:
					case 7:
					case 8:
						self->dirY = self->speed;

						self->action = &sinkAttackInit;
					break;

					default:
						self->dirY = self->speed;

						self->action = &changeSidesSink;
					break;
				}
			}
		}

		else if (self->mental == 30)
		{
			self->dirY = self->speed;

			self->action = &changeSidesSink;
		}

		else
		{
			self->action = &slimeAttackInit;
		}
	}

	checkToMap(self);
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
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 30)
		{
			self->element = SLIME;

			setEntityAnimation(self, "STAND");

			addClaws();

			e = addTemporaryItem("item/purifier_capsule", 0, 0, LEFT, 0, 0);

			e->fallout = &slimePillFallout;

			e->flags |= NO_DRAW;

			t = getTargetByName("PILL_TARGET_LEFT");

			if (t == NULL)
			{
				showErrorAndExit("Sewer Boss cannot find target");
			}

			e->x = t->x;
			e->y = t->y;

			e = addTemporaryItem("item/purifier_capsule", 0, 0, LEFT, 0, 0);

			e->fallout = &slimePillFallout;

			e->flags |= NO_DRAW;

			t = getTargetByName("PILL_TARGET_RIGHT");

			if (t == NULL)
			{
				showErrorAndExit("Sewer Boss cannot find target");
			}

			e->x = t->x;
			e->y = t->y;

			self->mental = 0;
		}

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
	Entity *temp;

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

	setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

	damage = 0;
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
		e->die = &clawDie;
		e->action = &clawWait;
		e->takeDamage = &clawTakeDamage;

		e->creditsAction = &clawWait;

		e->head = self;

		e->x = self->x + self->w / 2 - e->w / 2;

		e->startX = e->x;

		e->layer = (i == 0 ? BACKGROUND_LAYER : FOREGROUND_LAYER);

		e->thinkTime = (i == 0 ? 0 : 180);

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

	body = malloc(top->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Sewer Boss Arm...", top->mental * (int)sizeof(Entity *));
	}

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

		body[i]->creditsAction = &armPartWait;

		body[i]->type = ENEMY;

		body[i]->element = SLIME;

		body[i]->layer = top->layer;

		setEntityAnimation(body[i], "STAND");
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
	Entity *e;
	Target *t;

	self->face = self->head->face;

	self->x = self->head->x + self->head->w / 2 - self->w / 2;

	self->startX = self->x;

	self->thinkTime++;

	self->x = self->startX + sin(DEG_TO_RAD(self->thinkTime)) * 96;

	self->y = self->head->y + self->offsetY + 128;

	self->startY = self->head->y + self->offsetY + (self->layer == BACKGROUND_LAYER ? -16 : 0);

	checkToMap(self);

	alignArmToClaw();

	if (self->head->mental == -5)
	{
		setEntityAnimation(self, "WALK");

		self->targetX = self->face == LEFT ? self->startX - 64 : self->startX + 64;
		self->targetY = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->action = &punchAttackInit;

		self->thinkTime = self->layer == FOREGROUND_LAYER ? 30 : 60;

		self->endX = 15;
	}

	else if (self->head->mental == -10)
	{
		/* Only one grab at a time */

		if ((self->head->endX == 2 && self->layer == FOREGROUND_LAYER) || self->head->endX == 1)
		{
			setEntityAnimation(self, "WALK");

			self->targetX = self->startX;
			self->targetY = self->startY;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 4;
			self->dirY *= 4;

			self->action = &grabAttackInit;

			self->thinkTime = 30;
		}
	}

	else if (self->head->mental == -20)
	{
		setEntityAnimation(self, "WALK");

		if (self->layer == FOREGROUND_LAYER)
		{
			t = getTargetByName("PILL_TARGET_LEFT");

			self->face = RIGHT;
		}

		else
		{
			t = getTargetByName("PILL_TARGET_RIGHT");

			self->face = LEFT;
		}

		if (t == NULL)
		{
			showErrorAndExit("Sewer Boss cannot find target");
		}

		self->x = t->x;
		self->y = self->head->y;

		self->startY = self->y;
		self->startX = self->x;

		self->targetY = self->head->startY + self->offsetY;
		self->targetX = self->x;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->action = &clawSideAttackRise;

		self->thinkTime = 60;

		self->layer = FOREGROUND_LAYER;
	}

	else if (self->head->mental == -50)
	{
		setEntityAnimation(self, "WALK");

		self->face = LEFT;

		if (self->layer == FOREGROUND_LAYER)
		{
			t = getTargetByName("PILL_TARGET_RIGHT");

			self->face = LEFT;

			if (t == NULL)
			{
				showErrorAndExit("Sewer Boss cannot find target");
			}

			self->x = t->x;
			self->y = self->head->y;

			self->startY = self->y;
			self->startX = self->x;

			self->targetY = self->head->startY + self->offsetY;
			self->targetX = self->x;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 8;
			self->dirY *= 8;

			self->action = &finalAttackRise;

			self->die = &finalClawDie;

			self->fallout = &fallout;

			self->thinkTime = 0;

			self->damage = 0;
		}

		else
		{
			self->health = 0;

			self->inUse = FALSE;
		}
	}

	else if (self->head->mental == -65)
	{
		setEntityAnimation(self, "JUMP");

		if (self->layer == FOREGROUND_LAYER)
		{
			e = getEntityByObjectiveName("FLYING_MAGGOT_3");

			if (e == NULL)
			{
				showErrorAndExit("Sewer Boss cannot find Maggot");
			}

			e->touch = &entityTouch;

			self->x = e->x + e->w / 2 - self->w / 2;

			self->targetX = self->x;
			self->targetY = e->y + e->h / 2 - self->h / 2;

			self->action = &initialAttackRise;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 12;
			self->dirY *= 12;
		}

		else
		{
			self->health = 0;

			self->inUse = FALSE;
		}
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void clawSideAttackRise()
{
	if (atTarget())
	{
		self->thinkTime = self->face == LEFT ? 60 : 90;

		self->endX = self->head->health % 2 == 0 ? 5 : 10;

		self->action = &clawSideAttackWait;

		self->layer = BACKGROUND_LAYER;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void clawSideAttackWait()
{
	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->endX <= 0)
			{
				self->targetX = self->x;
				self->targetY = self->head->y;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 4;
				self->dirY *= 4;

				self->action = &clawSideAttackFinish;

				self->reactToBlock = NULL;
			}

			else
			{
				self->targetX = self->face == LEFT ? self->startX - 256 : self->startX + 256;
				self->targetY = self->y;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 12;
				self->dirY *= 12;

				self->action = &clawSideAttack;

				self->reactToBlock = &sideAttackReactToBlock;

				self->flags &= ~UNBLOCKABLE;
			}
		}
	}

	checkToMap(self);

	alignArmToClaw();
}

static void clawSideAttack()
{
	if (atTarget() || self->dirX == 0)
	{
		self->targetX = self->startX;
		self->targetY = self->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 12;
		self->dirY *= 12;

		self->action = &clawSideAttackWait;

		self->flags |= UNBLOCKABLE;

		self->reactToBlock = NULL;

		self->thinkTime = 60;

		self->endX--;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void sideAttackReactToBlock(Entity *other)
{
	self->dirX = 0;

	self->targetX = self->x;

	self->x = (int)self->x;
}

static void clawSideAttackFinish()
{
	if (atTarget())
	{
		setEntityAnimation(self, "STAND");

		self->layer = self->face == RIGHT ? FOREGROUND_LAYER : BACKGROUND_LAYER;

		self->head->mental--;

		self->action = &clawWait;

		self->thinkTime = self->layer == BACKGROUND_LAYER ? 180 : 0;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void punchAttackInit()
{
	if (atTarget())
	{
		if (self->head->mental == -5)
		{
			self->head->mental = -6;
		}

		else if (self->head->mental == -6)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->targetX = self->face == LEFT ? self->startX - 256 : self->startX + 256;
				self->targetY = self->y;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 12;
				self->dirY *= 12;

				self->flags &= ~UNBLOCKABLE;

				self->action = &punchAttack;

				self->reactToBlock = &punchReactToBlock;
			}
		}
	}

	checkToMap(self);

	alignArmToClaw();
}

static void punchReactToBlock(Entity *other)
{
	self->dirX = 0;

	self->targetX = self->x;

	self->x = (int)self->x;
}

static void punchAttack()
{
	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->endX--;

			if (self->endX <= 0)
			{
				self->targetX = self->startX;
				self->targetY = self->y;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 4;
				self->dirY *= 4;

				self->action = &punchAttackFinish;

				self->thinkTime = 60;
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
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (atTarget())
		{
			setEntityAnimation(self, "STAND");

			self->targetX = self->startX;

			self->targetY = self->head->y + self->offsetY + 128;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 4;
			self->dirY *= 4;

			self->action = &punchReturnToNormal;
		}

		checkToMap(self);
	}

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
			self->layer = FOREGROUND_LAYER;

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

		self->maxThinkTime = 7;
	}
}

static void grabWait()
{
	self->thinkTime++;

	self->y = self->endY + sin(DEG_TO_RAD(self->thinkTime)) * 8;

	player.x = self->x + self->w / 2 - player.w / 2;
	player.y = self->y + self->h / 2 - player.h / 2;

	alignArmToClaw();

	if (self->thinkTime >= 180)
	{
		self->targetX = self->x + (self->face == LEFT ? -128 : 128);
		self->targetY = self->startY - 64;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->action = &dropPlayer;

		self->thinkTime = 30;
	}

	else if (self->maxThinkTime <= 0)
	{
		self->action = &dropPlayer;

		self->thinkTime = 0;
	}
}

static void dropPlayer()
{
	if (atTarget() || self->maxThinkTime <= 0)
	{
		self->dirX = 0;
		self->dirY = 0;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->flags &= ~GRABBING;

			self->targetX = self->startX;
			self->targetY = self->startY;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 12;
			self->dirY *= 12;

			self->layer = self->head->endX == 1 ? BACKGROUND_LAYER : FOREGROUND_LAYER;

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

	if (self->element != WATER && prand() % 240 == 0)
	{
		x = self->x + self->w / 2 + ((prand() % 6) * (prand() % 2 == 0 ? -1 : 1));
		y = self->y + self->h - prand() % 10;

		e = addProjectile("enemy/slime_drip", self, x, y, 0, 0);

		e->element = SLIME;

		e->x -= e->w / 2;

		e->touch = NULL;
	}

	if (self->head->health <= 0)
	{
		self->dirX = 0;
		self->dirY = 0;

		self->action = &entityDieNoDrop;
		self->fallout = &entityDieNoDrop;
	}

	if (self->head->head->mental == -30)
	{
		self->draw = &armDraw;

		self->action = &armChangeToWater;
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
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
		self->touch = NULL;

		setEntityAnimation(self, self->element == SLIME ? "ATTACK_1" : "ATTACK_1_WATER");

		self->animationCallback = &slimeAttackBreatheIn;

		self->thinkTime = 180;

		self->mental = -15;
	}

	checkToMap(self);
}

static void slimeAttackBreatheIn()
{
	setEntityAnimation(self, self->element == SLIME ? "ATTACK_2" : "ATTACK_2_WATER");

	self->action = &slimeAttackBreatheIn;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 20;

		self->thinkTime = 30;

		self->action = &slimeAttack;
	}
}

static void slimeAttack()
{
	Entity *e;

	self->action = &slimeAttack;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->element == SLIME)
		{
			e = addProjectile("boss/borer_boss_slime", self, 0, 0, (self->face == RIGHT ? 7 : -7), 0);

			e->touch = &slimePlayer;

			e->damage = 3;
		}

		else
		{
			e = addProjectile("boss/sewer_boss_water", self, 0, 0, (self->face == RIGHT ? 7 : -7), 0);

			e->touch = &entityTouch;

			e->damage = 1;
		}

		if (self->face == LEFT)
		{
			e->x = self->target->x + self->target->w;
		}

		else
		{
			e->x = self->target->x;
		}

		e->y = self->target->y + self->target->h / 2 - e->h / 2;

		e->flags |= FLY;

		e->die = &slimeDie;

		e->targetX = e->x + (self->face == LEFT ? -SCREEN_WIDTH : SCREEN_WIDTH);
		e->targetY = e->y + (prand() % 2 == 0 ? -64 : 64);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 12;
		e->dirY *= (prand() % 3 == 0 ? 0 : 12);

		self->mental--;

		if (self->mental <= 0)
		{
			/* Keep attacking while the player still has arrows */

			if (self->health < 200 && getInventoryItemByObjectiveName("weapon/normal_arrow") != NULL)
			{
				self->touch = NULL;

				setEntityAnimation(self, self->element == SLIME ? "ATTACK_1" : "ATTACK_1_WATER");

				self->animationCallback = &slimeAttackBreatheIn;

				self->thinkTime = 60;

				self->mental = -15;
			}

			else
			{
				self->thinkTime = 60;

				self->action = &slimeAttackMouthClose;
			}
		}

		else
		{
			self->thinkTime = 10;
		}
	}
}

static void slimeAttackMouthClose()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, self->element == SLIME ? "ATTACK_3" : "ATTACK_3_WATER");

		self->animationCallback = &slimeAttackFinish;

		self->targetX = self->x;
		self->targetY = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 2;
		self->dirY *= 2;
	}
}

static void slimeAttackFinish()
{
	self->action = &slimeAttackFinish;

	self->touch = &entityTouch;

	if (atTarget())
	{
		self->mental = 30;

		self->action = &attackFinished;
	}

	setEntityAnimation(self, self->element == SLIME ? "STAND" : "STAND_WATER");

	checkToMap(self);
}

static void sinkAttackInit()
{
	checkToMap(self);

	if (self->y > self->endY)
	{
		self->y = self->endY;

		self->dirY = 0;

		self->thinkTime = 30;

		self->action = &sinkAttackWait;

		self->mental = -20;
	}
}

static void sinkAttackWait()
{
	int rand;
	Target *t;

	if ((self->mental == -21 && self->endX == 1) || (self->mental == -22 && self->endX == 2) || self->endX <= 0 || self->mental == -1)
	{
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
}

static void clawTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (damage != 0)
		{
			self->health -= damage;

			self->maxThinkTime--;

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
					setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
				}

				enemyPain();
			}

			else
			{
				self->head->endX--;

				self->head->mental = 0;

				self->damage = 0;

				self->thinkTime = 180;

				self->die();
			}
		}
	}
}

static void clawDie()
{
	Entity *arrow;

	/* Drop between 1 and 3 arrows */

	arrow = addTemporaryItem("weapon/normal_arrow", self->x, self->y, RIGHT, 0, ITEM_JUMP_HEIGHT);

	arrow->health = 1 + (prand() % 3);

	entityDieNoDrop();
}

static void slimeDie()
{
	playSoundToMap("sound/common/splat3", -1, self->x, self->y, 0);

	self->inUse = FALSE;
}

static void addMouthOrb()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sewer Boss Mouth Orb");
	}

	loadProperties("boss/sewer_boss_mouth_orb", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &orbWait;
	e->touch = entityTouch;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;

	self->target = e;
}

static void orbWait()
{
	Entity *e;

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

	if (self->head->health > 0)
	{
		self->takeDamage = self->head->endX != 0 ? NULL : &orbTakeDamage;
	}

	else
	{
		self->takeDamage = NULL;
	}

	if (self->head->mental == -15)
	{
		e = addSmoke(0, 0, "decoration/dust");

		if (e != NULL)
		{
			e->x = self->face == LEFT ? self->x - (128 + prand() % 64) : self->x + self->w + (128 + prand() % 64);
			e->y = self->y - 64;

			e->y += prand() % (self->h + 128);

			calculatePath(e->x, e->y, self->x + self->w / 2, self->y + self->h / 2, &e->dirX, &e->dirY);

			e->dirX *= 6;
			e->dirY *= 6;

			e->thinkTime = 20;
		}
	}
}

static void orbTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (damage != 0)
		{
			self->head->health -= damage;

			if (other->type == PROJECTILE)
			{
				other->inUse = FALSE;
			}

			if (self->head->health > 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);

				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				setCustomAction(self->head, &flashWhite, 6, 0, 0);

				setCustomAction(self->head, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				enemyPain();
			}

			else
			{
				self->head->mental = 0;

				setEntityAnimation(self->head, self->head->element == WATER ? "DIE_WATER" : "DIE");

				self->head->startX = self->head->x;

				self->head->targetX = 0;

				self->thinkTime = 180;

				self->head->action = &die;

				self->inUse = FALSE;
			}
		}
	}
}

static void slimePlayer(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;

		other->dirX = 0;

		setPlayerSlimed(120);

		self->die();
	}
}

static void slimePillFallout()
{
	if (self->environment == WATER)
	{
		self->action = &createSlime;

		self->thinkTime = 0;

		self->flags |= NO_DRAW;
	}
}

static void createSlime()
{
	fill(self->x / TILE_SIZE, self->y / TILE_SIZE);

	resetBlendTime();

	self->inUse = FALSE;
}

static void fill(int x, int y)
{
	if (mapTileAt(x, y) == WATER_TILE_START)
	{
		setTileAt(x, y, SLIME_TILE_BLEND_REVERSE);

		fill(x - 1, y);
		fill(x + 1, y);
		fill(x, y - 1);
		fill(x, y + 1);
	}
}

static void changeToWater()
{
	self->alpha--;

	if (self->alpha <= 0)
	{
		setEntityAnimation(self, "STAND_WATER");

		self->alpha = 255;

		self->action = &entityWait;

		self->draw = &drawLoopingAnimationToMap;

		self->mental = 0;
	}

	checkToMap(self);
}

static int draw()
{
	int frame, alpha;
	float timer;

	/* Draw the boss with its lowering alpha */

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	alpha = self->alpha;

	/* Draw the other part with its rising alpha */

	setEntityAnimation(self, "STAND_WATER");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = 255 - alpha;

	drawSpriteToMap();

	/* Reset back to original */

	setEntityAnimation(self, "STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = alpha;

	return 1;
}

static int armDraw()
{
	int frame, alpha;
	float timer;

	/* Draw the boss with its lowering alpha */

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	alpha = self->alpha;

	/* Draw the other part with its rising alpha */

	setEntityAnimation(self, "WALK");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = 255 - alpha;

	drawSpriteToMap();

	/* Reset back to original */

	setEntityAnimation(self, "STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = alpha;

	return 1;
}

static void armChangeToWater()
{
	self->alpha--;

	if (self->alpha <= 0)
	{
		self->element = WATER;

		setEntityAnimation(self, "WALK");

		self->alpha = 255;

		self->action = &armPartWait;

		self->draw = &drawLoopingAnimationToMap;
	}

	checkToMap(self);
}

static void die()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = 4;

		self->action = &dieSink;
	}

	else
	{
		self->x = self->startX + sin(DEG_TO_RAD(self->targetX)) * 4;

		self->targetX += 90;

		if (self->targetX >= 360)
		{
			self->targetX = 0;
		}
	}
}

static void dieSink()
{
	if (self->y >= self->endY)
	{
		self->y = self->endY;

		self->dirY = 0;

		self->thinkTime = 120;

		self->action = &finalAttack;

		addClaws();

		self->mental = -50;
	}

	checkToMap(self);
}


static void finalAttack()
{
	if (self->mental == -1)
	{
		entityDieVanish();
	}
}

static void finalAttackRise()
{
	if (atTarget())
	{
		self->action = &finalGrabPlayer;

		self->touch = &grabTouch;

		self->layer = BACKGROUND_LAYER;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void finalGrabPlayer()
{
	if (self->flags & GRABBING)
	{
		self->touch = &entityTouch;

		self->action = &finalGrabWait;

		self->endY = self->y - 16;
	}

	else
	{
		self->dirX = player.x < self->x ? -6 : 6;
		self->dirY = player.y < self->y ? -6 : 6;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void finalGrabWait()
{
	self->thinkTime++;

	self->y = self->endY + sin(DEG_TO_RAD(self->thinkTime)) * 8;

	player.x = self->x + self->w / 2 - player.w / 2;
	player.y = self->y + self->h / 2 - player.h / 2;

	alignArmToClaw();

	if (self->thinkTime >= 360)
	{
		self->takeDamage = NULL;

		player.fallout = &fallout;

		self->targetX = self->startX;
		self->targetY = self->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->action = &dragDownInit;

		self->thinkTime = 60;
	}
}

static void dragDownInit()
{
	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->targetX = self->startX;
			self->targetY = self->startY + 64;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 8;
			self->dirY *= 8;

			self->action = &dragDown;

			self->thinkTime = 60;
		}
	}

	player.x = self->x + self->w / 2 - player.w / 2;
	player.y = self->y + self->h / 2 - player.h / 2;

	checkToMap(self);

	alignArmToClaw();
}

static void dragDown()
{
	if (atTarget())
	{
		removeInventoryItemByObjectiveName("Amulet of Resurrection");

		player.die();

		self->health = 0;

		self->inUse = FALSE;
	}

	player.x = self->x + self->w / 2 - player.w / 2;
	player.y = self->y + self->h / 2 - player.h / 2;

	checkToMap(self);

	alignArmToClaw();
}

static void finalClawDie()
{
	self->action = &finalClawDie;

	self->thinkTime--;

	self->dirX = 0;

	if (self->thinkTime <= 0)
	{
		self->head->mental = -1;

		clearContinuePoint();

		freeBossHealthBar();

		fadeBossMusic();

		entityDieNoDrop();
	}

	self->flags &= ~FLY;

	checkToMap(self);
}

static void initialAttackRise()
{
	if (atTarget())
	{
		self->action = &grabMaggot;

		self->touch = &maggotTouch;

		self->layer = FOREGROUND_LAYER;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void maggotTouch(Entity *other)
{
	if (strcmpignorecase(other->objectiveName, "FLYING_MAGGOT_3") == 0)
	{
		playSoundToMap("sound/boss/armour_boss/tongue_start", BOSS_CHANNEL, self->x, self->y, 0);

		other->action = &struggle;

		self->head = other;

		self->thinkTime = 120;

		self->flags |= GRABBING;

		self->touch = &entityTouch;
	}
}

static void grabMaggot()
{
	if (self->flags & GRABBING)
	{
		self->touch = &entityTouch;

		self->action = &maggotGrabWait;

		self->thinkTime = 90;
	}

	checkToMap(self);

	alignArmToClaw();
}

static void maggotGrabWait()
{
	self->head->x = self->x + self->w / 2 - self->head->w / 2;
	self->head->y = self->y + self->h / 2 - self->head->h / 2;

	alignArmToClaw();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->startX;
		self->targetY = self->startY + 64;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 12;
		self->dirY *= 12;

		self->action = &dragDown;

		self->action = &maggotGrabSink;

		self->fallout = &fallout;

		self->head->fallout = &fallout;
	}
}

static void maggotGrabSink()
{
	if (atTarget())
	{
		self->head->inUse = FALSE;

		self->health = 0;

		self->inUse = FALSE;
	}

	self->head->x = self->x + self->w / 2 - self->head->w / 2;
	self->head->y = self->y + self->h / 2 - self->head->h / 2;

	checkToMap(self);

	alignArmToClaw();
}

static void struggle()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->thinkTime = 5;
	}
}

static void addSlimeDrips()
{
	int i;
	Entity *e;

	for (i=0;i<20;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Sewer Boss Slime Drip");
		}

		loadProperties("boss/sewer_boss_slime_drip", e);

		e->draw = &drawLoopingAnimationToMap;
		e->action = &dripWait;

		e->creditsAction = &dripWait;

		e->head = self;

		setEntityAnimationByID(e, i);

		e->mental = i;

		e->currentFrame = prand() % getFrameCount(e);
	}
}

static void dripWait()
{
	int frame;
	float timer;

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

	if (self->head->mental == -30)
	{
		self->draw = &dripDraw;

		self->action = &dripChangeToWater;
	}

	if (self->element == WATER && self->head->element != WATER)
	{
		self->element = SLIME;

		frame = self->currentFrame;
		timer = self->frameTimer;

		setEntityAnimationByID(self, self->mental);

		self->currentFrame = frame;
		self->frameTimer = timer;
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static int dripDraw()
{
	int frame, alpha;
	float timer;

	/* Draw the boss with its lowering alpha */

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	alpha = self->alpha;

	/* Draw the other part with its rising alpha */

	setEntityAnimationByID(self, self->mental + 20);

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = 255 - alpha;

	drawSpriteToMap();

	/* Reset back to original */

	setEntityAnimationByID(self, self->mental);

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = alpha;

	return 1;
}

static void dripChangeToWater()
{
	int frame;
	float timer;

	self->alpha--;

	if (self->alpha <= 0)
	{
		self->element = WATER;

		frame = self->currentFrame;
		timer = self->frameTimer;

		setEntityAnimationByID(self, self->mental + 20);

		self->currentFrame = frame;
		self->frameTimer = timer;

		self->alpha = 255;

		self->action = &dripWait;

		self->draw = &drawLoopingAnimationToMap;
	}

	checkToMap(self);
}

static void creditsMove()
{
	if (self->health != -1)
	{
		addSlimeDrips();

		addClaws();

		self->health = -1;
	}

	bossMoveToMiddle();

	alignArmToClaw();
}
