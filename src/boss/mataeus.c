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
#include "../enemy/rock.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/gib.h"
#include "../hud.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static int bodyParts = 20;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void entityWait(void);
static void hover(void);
static void attackFinished(void);
static void knifeThrowInit(void);
static void knifeWait(void);
static void knifeAttack(void);
static void knifeBlock(Entity *);
static void specialKnifeBlock(Entity *);
static void knifeBlockWait(void);
static void specialKnifeBlockWait(void);
static void takeDamage(Entity *, int);
static void knifeThrow(void);
static void ceilingDropInit(void);
static void ceilingDropWait(void);
static void knifeDie(void);
static void verticalKnifeThrowInit(void);
static void verticalKnifeWait(void);
static void lavaCeilingDropWait(void);
static void lavaCeilingDropInit(void);
static void lavaCeilingDropEnd(void);
static void stunned(void);
static void stunFinish(void);
static void resetCeiling(void);
static void dropAnchor(void);
static void alignChainToAnchor(void);
static void anchorTakeDamage(Entity *, int);
static void chainTakeDamage(Entity *, int);
static void chainWait(void);
static void anchorWait(void);
static void anchorInGround(void);
static void anchorDie(void);
static void openRift(void);
static void openRiftWait(void);
static void riftRise(void);
static void riftAttract(void);
static void riftSink(void);
static void riftKnifeThrowInit(void);
static void riftKnifeThrow(void);
static void riftTouch(Entity *);
static void riftKillBoss(void);
static int isAnchored(void);
static void die(void);
static void addRiftEnergy(int, int);
static void energyMoveToRift(void);
static void dieWait(void);
static void riftDestroyWait(void);
static void gibWait(void);
static void createShield(void);
static void shieldWait(void);
static void horizontalKnifeThrowInit(void);
static void horizontalKnifeWait(void);
static void horizontalKnifeThrowWait(void);
static void creditsMove(void);

Entity *addMataeus(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Mataeus");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;
	e->pain = &enemyPain;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "CUSTOM_1");

	return e;
}

static void initialise()
{
	self->thinkTime = 180;

	setEntityAnimation(self, "CUSTOM_1");

	self->action = &doIntro;

	self->startX = self->x;
	self->startY = self->y;

	self->endX = self->x;
	self->endY = self->y;

	setContinuePoint(FALSE, self->name, NULL);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "CUSTOM_2");

		self->animationCallback = &introPause;
	}
}

static void introPause()
{
	self->dirX = 0.5;

	checkToMap(self);

	playDefaultBossMusic();

	initBossHealthBar();

	self->action = &createShield;

	self->thinkTime = 90;

	self->flags |= LIMIT_TO_SCREEN;

	facePlayer();

	self->startY = self->y;
}

static void attackFinished()
{
	self->frameSpeed = 1;

	self->dirX = 0;

	self->thinkTime = 90;

	self->damage = 1;

	self->mental = 0;

	self->action = &entityWait;

	self->touch = &entityTouch;

	self->takeDamage = &takeDamage;

	setEntityAnimation(self, "STAND");

	hover();
}

static void entityWait()
{
	int i;

	self->dirX = 0;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->maxThinkTime = 0;

		if (self->health > 1500)
		{
			i = prand() % 2;

			switch (i)
			{
				case 0:
					self->action = &ceilingDropInit;
				break;

				default:
					self->action = &knifeThrowInit;
				break;
			}
		}

		else if (self->health > 500)
		{
			i = prand() % 3;

			switch (i)
			{
				case 0:
					self->action = &lavaCeilingDropInit;
				break;

				case 1:
					self->maxThinkTime = 3 + prand() % 3;

					self->action = &verticalKnifeThrowInit;
				break;

				default:
					self->action = &knifeThrowInit;
				break;
			}
		}

		else
		{
			if (isAnchored() == FALSE)
			{
				self->action = &dropAnchor;
			}

			else
			{
				i = prand() % 5;

				switch (i)
				{
					case 0:
					case 1:
						self->action = &knifeThrowInit;
					break;

					case 2:
					case 3:
						self->action = &horizontalKnifeThrowInit;
					break;

					default:
						self->action = &openRift;
					break;
				}
			}
		}
	}

	hover();
}

static void hover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;
}

static void knifeThrowInit()
{
	int i, radians;
	Entity *e;

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}

		radians = prand() % 7;

		if (radians == 0)
		{
			loadProperties("boss/mataeus_knife_special", e);

			e->reactToBlock = &specialKnifeBlock;
		}

		else
		{
			loadProperties("boss/mataeus_knife", e);

			e->reactToBlock = &knifeBlock;
		}

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->flags |= LIMIT_TO_SCREEN;

		radians = DEG_TO_RAD(i * 60);

		e->x += (0 * cos(radians) - 0 * sin(radians));
		e->y += (0 * sin(radians) + 0 * cos(radians));

		e->action = &knifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		e->health = radians;

		e->head = self;

		setEntityAnimation(e, "WALK");
	}

	self->mental = 0;

	playSoundToMap("sound/boss/mataeus/create_knife", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 60;

	self->action = &knifeThrow;
}

static void verticalKnifeThrowInit()
{
	int i, startX, startY, knives;
	Entity *e;

	startX = getMapStartX();
	startY = getMapStartY() + 96;

	knives = self->maxThinkTime % 2 == 0 ? 7 : 6;

	for (i=0;i<knives;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}

		loadProperties("boss/mataeus_knife", e);

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->targetX = startX + (i * 96) + (self->maxThinkTime % 2 == 0 ? 8 : 56);
		e->targetY = startY;

		e->action = &verticalKnifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 30;
		e->dirY *= 30;

		e->head = self;

		setEntityAnimation(e, "WALK");
	}

	self->mental = 0;

	playSoundToMap("sound/boss/mataeus/create_knife", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 60;

	self->action = &knifeThrow;
}

static void knifeThrow()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			playSoundToMap("sound/boss/mataeus/throw_knife", -1, self->x, self->y, 0);

			self->mental = 1;
		}

		else
		{
			self->maxThinkTime--;

			self->action = self->maxThinkTime > 0 ? &verticalKnifeThrowInit : &attackFinished;
		}
	}

	hover();
}

static void verticalKnifeWait()
{
	if (atTarget())
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		if (self->head->mental == 1)
		{
			self->thinkTime = 30;

			self->action = &knifeAttack;

			self->dirX = 0.01;
			self->dirY = 16;

			self->startX = self->dirX;
			self->startY = self->dirY;

			self->thinkTime = 60;

			setEntityAnimation(self, "ATTACK_2");

			self->health = 1;
		}
	}

	checkToMap(self);
}

static void knifeWait()
{
	float startX, startY, endX, endY;

	self->mental += 4;

	if (self->mental >= 64)
	{
		self->mental = 64;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	self->x += (self->mental * cos(self->health) - 0 * sin(self->health));
	self->y += (self->mental * sin(self->health) + 0 * cos(self->health));

	if (self->head->mental == 1)
	{
		self->thinkTime = 30;

		self->action = &knifeAttack;

		startX = self->x;
		startY = self->y;

		endX = player.x + player.w / 2;
		endY = player.y;

		calculatePath(startX, startY, endX, endY, &self->dirX, &self->dirY);

		self->dirX *= 30;
		self->dirY *= 30;

		self->startX = self->dirX;
		self->startY = self->dirY;

		self->thinkTime = 60;

		facePlayer();

		setEntityAnimation(self, "ATTACK_1");

		self->health = 1;
	}

	else if (self->head->flags & HELPLESS)
	{
		entityDieNoDrop();
	}
}

static void knifeAttack()
{
	checkToMap(self);

	if (self->dirX == 0 || self->dirY == 0)
	{
		self->damage = 0;

		self->flags |= FLY;

		self->dirX = 0;

		self->dirY = 0;

		self->thinkTime = 15;

		self->action = &knifeDie;
	}
}

static void knifeDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void specialKnifeBlock(Entity *other)
{
	self->action = &specialKnifeBlockWait;

	self->activate = &throwItem;
}

static void knifeBlock(Entity *other)
{
	self->action = &knifeBlockWait;

	self->activate = &throwItem;
}

static void specialKnifeBlockWait()
{
	if (self->flags & FLY)
	{
		self->flags &= ~FLY;

		self->dirX = self->x < player.x ? -5 : 5;

		self->dirY = -5;

		self->thinkTime = 120;

		self->damage = 50;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;

		self->dirX = 0;

		self->action = &generalItemAction;

		self->touch = &keyItemTouch;

		self->activate = &throwItem;
	}
}

static void knifeBlockWait()
{
	if (self->flags & FLY)
	{
		self->flags &= ~FLY;

		self->dirX = self->x < player.x ? -5 : 5;

		self->dirY = -5;

		self->thinkTime = 120;

		self->damage = 50;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->action = prand() % 10 == 0 ? entityDie : entityDieNoDrop;

		self->dirX = 0;
	}
}

static void ceilingDropInit()
{
	EntityList *list = getEntitiesByObjectiveName("MATAEUS_CEILING");
	EntityList *l;
	Entity *e;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		if (e->mental == 0)
		{
			e->thinkTime = prand() % 1200;

			e->mental = 1;
		}
	}

	freeEntityList(list);

	self->thinkTime = 60;

	self->action = &ceilingDropWait;

	hover();
}

static void ceilingDropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void lavaCeilingDropInit()
{
	EntityList *list = getEntitiesByObjectiveName("MATAEUS_CEILING");
	EntityList *l;
	Entity *e;

	self->mental = 0;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		e->head = self;

		if (e->mental == 0)
		{
			e->thinkTime = prand() % 300;

			e->mental = 2;
		}

		self->mental++;
	}

	freeEntityList(list);

	self->thinkTime = 1200;

	self->action = &lavaCeilingDropWait;

	hover();
}

static void lavaCeilingDropWait()
{
	EntityList *list;
	EntityList *l;
	Entity *e;
	int i = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		list = getEntitiesByObjectiveName("MATAEUS_CEILING");

		for (l=list->next;l!=NULL;l=l->next)
		{
			e = l->entity;

			e->mental = 3;

			e->head = self;

			i++;
		}

		self->thinkTime = 60;

		self->action = &lavaCeilingDropEnd;

		self->mental = i;
	}

	hover();
}

static void lavaCeilingDropEnd()
{
	if (self->mental <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}

	hover();
}

static void takeDamage(Entity *other, int damage)
{
	int health;
	Entity *temp;

	if (self->health <= 1500 || (self->flags & HELPLESS))
	{
		if (self->flags & INVULNERABLE)
		{
			return;
		}

		/* Take minimal damage from bombs */

		if (other->type == EXPLOSION)
		{
			damage = 1;
		}

		if (damage != 0)
		{
			/* Don't die from regular hits */

			self->health -= damage;

			if (self->health <= 0)
			{
				self->health = 1;
			}

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
		if (strcmpignorecase(other->name, "boss/mataeus_knife_special") != 0)
		{
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

		else
		{
			health = self->health;

			self->health -= damage;

			setCustomAction(self, &flashWhite, 6, 0, 0);

			setCustomAction(self->target, &flashWhite, 6, 0, 0);

			other->inUse = FALSE;

			enemyPain();

			if ((health > 1500 && self->health <= 1500))
			{
				self->flags &= ~FLY;

				self->action = &stunned;

				resetCeiling();
			}
		}
	}
}

static void stunned()
{
	int i;
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		setCustomAction(self, &helpless, 300, 0, 0);

		if (onGround == 0)
		{
			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);
		}

		self->action = &stunFinish;
	}
}

static void stunFinish()
{
	self->flags |= FLY;

	self->dirY = -6;

	self->startX = 0;

	checkToMap(self);

	if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->action = &attackFinished;
	}
}

static void resetCeiling()
{
	EntityList *list = getEntitiesByObjectiveName("MATAEUS_CEILING");
	EntityList *l;
	Entity *e;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		e->mental = 0;
	}

	freeEntityList(list);
}

static void dropAnchor()
{
	Entity **body, *anchor;
	int i;

	body = malloc(bodyParts * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Mataeus Anchor chain...", bodyParts * (int)sizeof(Entity *));
	}

	for (i=bodyParts-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Chain");
		}

		loadProperties("boss/mataeus_anchor_chain", body[i]);

		body[i]->x = self->x;
		body[i]->y = self->y;

		body[i]->action = &chainWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;
		body[i]->takeDamage = &chainTakeDamage;

		body[i]->type = ENEMY;

		setEntityAnimation(body[i], "STAND");
	}

	/* Create the anchor so that it's on top */

	anchor = getFreeEntity();

	if (anchor == NULL)
	{
		showErrorAndExit("No free slots to add Mataeus Anchor");
	}

	loadProperties("boss/mataeus_anchor", anchor);

	anchor->action = &anchorWait;

	anchor->draw = &drawLoopingAnimationToMap;
	anchor->touch = NULL;
	anchor->die = &anchorDie;
	anchor->takeDamage = &anchorTakeDamage;
	anchor->pain = &enemyPain;

	anchor->type = ENEMY;

	setEntityAnimation(anchor, "STAND");

	anchor->x = self->x + self->w / 2 - anchor->w / 2;
	anchor->y = self->y + self->h / 2 - anchor->h / 2;

	anchor->endX = anchor->x;
	anchor->endY = anchor->y;

	/* Link the sections */

	for (i=bodyParts-1;i>=0;i--)
	{
		if (i == 0)
		{
			anchor->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];
		}

		body[i]->head = anchor;

		body[i]->x = anchor->x + anchor->w / 2 - body[i]->w / 2;
	}

	anchor->head = self;

	self->target = anchor;

	self->action = &attackFinished;
}

static void alignChainToAnchor()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->x + self->w / 2;
	y = self->y;

	self->endX = self->head->x + self->head->w / 2;

	partDistanceX = fabs(self->endX - x);
	partDistanceY = fabs(self->endY - self->y);

	partDistanceX /= bodyParts;
	partDistanceY /= bodyParts;

	e = self->target;

	while (e != NULL)
	{
		x -= partDistanceX;
		y -= partDistanceY;

		e->x = (e->target == NULL ? self->endX : x - e->w / 2);
		e->y = (e->target == NULL ? self->endY : y);

		e->damage = self->damage;

		e->face = self->face;

		if (self->flags & NO_DRAW)
		{
			e->flags |= NO_DRAW;
		}

		else
		{
			e->flags &= ~NO_DRAW;
		}

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

static void anchorTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		if (strcmpignorecase(other->name, "boss/mataeus_knife_special") != 0)
		{
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

		else
		{
			self->health -= damage;

			setCustomAction(self, &flashWhite, 6, 0, 0);

			other->inUse = FALSE;

			enemyPain();

			if (self->health <= 0)
			{
				self->die();
			}
		}
	}
}

static void chainTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	temp = self;

	self = self->head;

	self->takeDamage(other, damage);

	self = temp;
}

static void chainWait()
{
	checkToMap(self);
}

static void anchorWait()
{
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->targetY = self->y + self->h / 2;

		playSoundToMap("sound/common/crumble", BOSS_CHANNEL, self->x, self->y, 0);

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

		self->action = &anchorInGround;
	}

	self->endY = self->head->y + self->head->h / 2 - self->h / 2;

	alignChainToAnchor();
}

static void anchorInGround()
{
	self->y += 3;

	if (self->y >= self->targetY)
	{
		self->y = self->targetY;
	}

	self->endY = self->head->y + self->head->h / 2 - self->h / 2;

	alignChainToAnchor();
}

static void anchorDie()
{
	Entity *e;

	e = self;

	playSoundToMap("sound/enemy/centurion/centurion_die", -1, self->x, self->y, 0);

	self = self->target;

	while (self != NULL)
	{
		self->die();

		self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		self->dirY = ITEM_JUMP_HEIGHT;

		self = self->target;
	}

	self = e;

	self->die = &entityDieVanish;

	self->die();

	self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
	self->dirY = ITEM_JUMP_HEIGHT;
}

static void openRift()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Rift");
	}

	loadProperties("boss/mataeus_rift", e);

	e->action = &riftRise;

	e->draw = &drawLoopingAnimationToMap;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h / 2 - e->h / 2;

	self->mental = 0;

	self->action = &openRiftWait;

	e->head = self;

	e->targetX = getMapStartX() + SCREEN_WIDTH / 2 - e->w / 2;
	e->targetY = getMapFloor(e->x, e->y);

	e->startY = e->targetY - e->h;

	e->endY = e->targetY;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

	e->health = -1;

	playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 120;

	self->targetX = e->targetX + e->w / 2 - self->w / 2;
	self->targetY = e->startY + e->h / 2 - self->h / 2;
}

static void openRiftWait()
{
	if (self->flags & ATTRACTED)
	{
		if (isAnchored() == FALSE)
		{
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 6;
			self->dirY *= 6;

			self->face = RIGHT;

			self->thinkTime = 30;

			self->action = &die;
		}

		else
		{
			if (self->x <= self->endX - 64)
			{
				self->x = self->endX - 64;
			}

			if (player.health > 0)
			{
				self->action = &riftKnifeThrowInit;
			}
		}
	}

	else
	{
		self->dirX = 1;

		if (fabs(self->x - self->endX) <= fabs(self->dirX))
		{
			self->dirX = 0;

			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->action = &attackFinished;
			}
		}
	}

	checkToMap(self);

	hover();
}

static void riftKnifeThrowInit()
{
	int i, radians;
	Entity *e;

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}

		loadProperties("boss/mataeus_knife", e);

		e->reactToBlock = &knifeBlock;

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		radians = DEG_TO_RAD(i * 60);

		e->x += (0 * cos(radians) - 0 * sin(radians));
		e->y += (0 * sin(radians) + 0 * cos(radians));

		e->action = &knifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		e->health = radians;

		e->head = self;

		setEntityAnimation(e, "WALK");
	}

	self->mental = 0;

	playSoundToMap("sound/boss/mataeus/create_knife", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 60;

	self->action = &riftKnifeThrow;

	if (self->x <= self->endX - 64)
	{
		self->x = self->endX - 64;
	}

	checkToMap(self);

	hover();
}

static void riftKnifeThrow()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			playSoundToMap("sound/boss/mataeus/throw_knife", -1, self->x, self->y, 0);

			self->mental = 1;
		}

		else
		{
			self->thinkTime = 30;

			self->action = &openRiftWait;
		}
	}

	if (self->x <= self->endX - 64)
	{
		self->x = self->endX - 64;
	}

	checkToMap(self);

	hover();
}

static void riftRise()
{
	if (self->health == -1)
	{
		self->health = playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);
	}

	self->y -= 0.5;

	self->head->thinkTime = 120;

	if (self->y <= self->startY)
	{
		stopSound(self->health);

		self->health = playSoundToMap("sound/item/rift", -1, self->x, self->y, -1);

		self->y = self->startY;

		self->thinkTime = 300;

		setEntityAnimation(self, "WALK");

		self->touch = &riftTouch;

		self->head->mental = 1;

		self->action = &riftAttract;

		setCustomAction(&player, &attract, 2, 0, 0);
	}

	addSmoke(self->x + prand() % self->w, self->endY - prand() % 10, "decoration/dust");
}

static void riftAttract()
{
	Entity *temp;

	setCustomAction(self->head, &attract, 2, 0, -2.5);

	setCustomAction(&player, &attract, 2, 0, (player.x < (self->x + self->w / 2) ? (player.speed - 0.5) : -(player.speed - 0.5)));

	self->thinkTime--;

	temp = self;

	self = temp->head;

	if (isAnchored() == FALSE)
	{
		temp->thinkTime = 60000;
	}

	self = temp;

	if (self->thinkTime <= 0)
	{
		self->touch = NULL;

		setEntityAnimation(self, "STAND");

		stopSound(self->health);

		self->health = -1;

		self->action = &riftSink;

		self->damage = 0;
	}

	if (prand() % 3 == 0)
	{
		addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
	}

	checkToMap(self);
}

static void riftSink()
{
	if (self->health == -1)
	{
		self->health = playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);
	}

	self->y += 0.5;

	if (self->y >= self->endY)
	{
		stopSound(self->health);

		self->inUse = FALSE;
	}

	addSmoke(self->x + prand() % self->w, self->endY - prand() % 10, "decoration/dust");
}

static void riftTouch(Entity *other)
{
	int i;
	Entity *temp, *e;
	EntityList *list, *l;

	i = 0;

	if (other->type == PLAYER && player.health > 0)
	{
		temp = self;

		self = other;

		list = playerGib();

		for (l=list->next;l!=NULL;l=l->next)
		{
			e = l->entity;

			e->head = temp;

			e->x = temp->x + temp->w / 2 - e->w / 2;
			e->y = temp->y + temp->h / 2 - e->h / 2;

			e->startX = e->x;
			e->startY = e->y;

			e->mental = 0;

			e->health = prand() % 360;

			e->dirX = 1;

			e->action = &gibWait;

			e->thinkTime = 180;

			i++;
		}

		freeEntityList(list);

		self = temp;

		self->action = &riftDestroyWait;

		self->mental = i;
	}

	else if (strcmpignorecase(other->name, "boss/mataeus") == 0)
	{
		temp = self;

		/* Mataeus */

		self = other;

		list = throwGibs("boss/mataeus_gibs", 11);

		for (l=list->next;l!=NULL;l=l->next)
		{
			e = l->entity;

			e->head = temp;

			e->x = temp->x + temp->w / 2 - e->w / 2;
			e->y = temp->y + temp->h / 2 - e->h / 2;

			e->startX = e->x;
			e->startY = e->y;

			e->mental = 0;

			e->health = prand() % 360;

			e->dirX = 1;

			e->action = &gibWait;

			e->thinkTime = 180;

			i++;
		}

		freeEntityList(list);

		self->health = 1;

		self->inUse = TRUE;

		self->action = &doNothing;

		self->flags |= NO_DRAW;

		self->touch = NULL;

		/* Rift */

		self = temp;

		self->mental = i;

		self->target = other;

		self->touch = NULL;

		self->thinkTime = 120;

		self->action = &riftKillBoss;
	}
}

static void riftKillBoss()
{
	checkToMap(self);

	if (self->mental <= 0)
	{
		self->target->health = 0;

		self->target->action = &dieWait;

		self->target->thinkTime = 420;

		self->touch = NULL;

		setEntityAnimation(self, "STAND");

		stopSound(self->health);

		self->health = -1;

		self->action = &riftSink;

		self->damage = 0;
	}

	checkToMap(self);
}

static int isAnchored()
{
	return (self->target != NULL && self->target->health > 0 && strcmpignorecase(self->target->name, "boss/mataeus_anchor") == 0);
}

static void die()
{
	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->thinkTime--;

	if (self->thinkTime < -300)
	{
		self->dirX *= 12;
		self->dirY *= 12;
	}

	else if (self->thinkTime <= 0 && self->thinkTime >= -120)
	{
		self->dirX *= -0.25;
		self->dirY *= -0.25;
	}

	else if (self->thinkTime <= -150 && self->thinkTime >= -270)
	{
		self->dirX *= -0.25;
		self->dirY *= -0.25;
	}

	else if (self->thinkTime <= -270 && self->thinkTime >= -300)
	{
		self->dirX *= 0;
		self->dirY *= 0;
	}

	else
	{
		self->dirX *= 6;
		self->dirY *= 6;
	}

	if (atTarget())
	{
		self->dirX = 0;
		self->dirY = 0;

		self->health = 0;

		self->action = &dieWait;

		self->flags |= NO_DRAW;

		self->damage = 0;

		self->thinkTime = 450;
	}

	checkToMap(self);
}

static void addRiftEnergy(int x, int y)
{
	Entity *e;

	e = addBasicDecoration(x, y, "decoration/rift_energy");

	if (e != NULL)
	{
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

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		fadeBossMusic();

		entityDieVanish();
	}
}

static void riftDestroyWait()
{
	checkToMap(self);

	if (self->mental <= 0)
	{
		setEntityAnimation(self, "STAND");

		stopSound(self->health);

		self->health = -1;

		self->action = &riftSink;
	}
}

static void gibWait()
{
	float radians;

	if (self->dirX == -1)
	{
		self->mental -= 2;

		if (self->mental <= 0)
		{
			self->head->mental--;

			self->inUse = FALSE;
		}
	}

	else
	{
		self->mental += 2;

		if (self->mental >= 160)
		{
			self->mental = 160;

			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->dirX = -1;
			}
		}
	}

	self->health += 8;

	radians = DEG_TO_RAD(self->health);

	self->x = (0 * cos(radians) - self->mental * sin(radians));
	self->y = (0 * sin(radians) + self->mental * cos(radians));

	self->x += self->startX;
	self->y += self->startY;
}

static void createShield()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Mataeus' Shield");
	}

	loadProperties("boss/mataeus_shield", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &shieldWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->head = self;

	e->alpha = 128;

	self->target = e;

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h / 2 - e->h / 2;

	self->action = &attackFinished;
}

static void shieldWait()
{
	float radians;

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	if (self->head->health <= 1500)
	{
		self->inUse = FALSE;
	}

	self->thinkTime += 5;

	radians = DEG_TO_RAD(self->thinkTime);

	self->alpha = 128 + (64 * cos(radians));
}

static void horizontalKnifeThrowInit()
{
	int i, mapFloor;
	Entity *e;

	mapFloor = getMapFloor(self->x, self->y);

	for (i=0;i<10;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}

		loadProperties("boss/mataeus_knife", e);

		setEntityAnimation(e, "WALK");

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->targetX = getMapStartX() + (i % 2 == 0 ? 5 : SCREEN_WIDTH - e->w - 5);
		e->targetY = mapFloor - 48;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 30;
		e->dirY *= 30;

		e->action = &horizontalKnifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		e->reactToBlock = &knifeBlock;

		e->health = 0;

		e->thinkTime = 180 + 45 * i;

		e->head = self;

		self->maxThinkTime--;

		playSoundToMap("sound/boss/mataeus/create_knife", BOSS_CHANNEL, self->x, self->y, 0);

		self->thinkTime = e->thinkTime;

		self->action = &horizontalKnifeThrowWait;
	}
}

static void horizontalKnifeThrowWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);

	hover();
}

static void horizontalKnifeWait()
{
	self->thinkTime--;

	if (self->health == 0)
	{
		if (atTarget())
		{
			self->dirX = 0;
			self->dirY = 0;

			self->thinkTime--;
		}

		if (self->thinkTime <= 0)
		{
			self->action = &knifeAttack;

			facePlayer();

			self->dirX = (self->face == RIGHT ? 25 : -25);
			self->dirY = 0.01;

			self->thinkTime = 60;

			setEntityAnimation(self, "ATTACK_3");

			self->health = 1;

			playSoundToMap("sound/boss/mataeus/throw_knife", -1, self->x, self->y, 0);
		}
	}

	checkToMap(self);
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->creditsAction = &bossMoveToMiddle;
}
