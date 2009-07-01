/*
Copyright (C) 2009 Parallel Realities

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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../item/key_items.h"
#include "../world/target.h"
#include "../graphics/decoration.h"
#include "../player.h"
#include "../geometry.h"
#include "../hud.h"
#include "../enemy/baby_slime.h"
#include "../projectile.h"

extern Entity *self, player;

static int bodyParts = 10;
static Entity *headPart;

static void bodyWait(void);
static void initialise(void);
static void headWait(void);
static void riseUp(void);
static void createBody(void);
static void setSnakePosition(int, int);
static void alignBodyToHead(void);
static void biteAttackInit(void);
static void biteAttackWindUp(void);
static void biteAttack(void);
static void returnToStart(void);
static void attackFinished(void);
static void die(void);
static void dieWait(void);
static void fallToGround(void);
static void changeSidesInit(void);
static void changeSides(void);
static void riseUpWait(void);
static void takeDamage(Entity *, int);
static void shotAttackInit(void);
static void shotAttackWindUp(void);
static void shotAttack(void);
static void specialShotWait(void);
static void specialShotBlock(void);
static void specialShotTouch(Entity *);
static void biteReactToBlock(void);
static void crushAttackInit(void);
static void crushAttackMoveToPosition(void);
static void crushAttack(void);
static void crushAttackFinish(void);
static void addSmokeAlongBody(void);
static void stunned(void);
static void crushAttackHit(Entity *);
static void bodyTakeDamage(Entity *, int);

Entity *addSnakeBoss(int x, int y, char *name)
{
	Entity *head;

	head = getFreeEntity();

	if (head == NULL)
	{
		printf("No free slots to add a Snake Boss\n");

		exit(1);
	}

	loadProperties(name, head);

	head->x = x;
	head->y = y;

	head->action = &initialise;

	head->draw = &drawLoopingAnimationToMap;
	head->touch = &entityTouch;
	head->die = &die;
	head->takeDamage = &takeDamage;

	head->type = ENEMY;

	head->active = FALSE;

	setEntityAnimation(head, STAND);

	return head;
}

static void bodyWait()
{
	checkToMap(self);
}

static void initialise()
{
	int minX, minY;
	Target *t;

	minX = getMapStartX();
	minY = getMapStartY();

	self->flags |= NO_DRAW;

	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			createBody();

			centerMapOnEntity(NULL);

			t = getTargetByName("SNAKE_BOSS_TARGET_RIGHT");

			if (t == NULL)
			{
				printf("Snake boss is missing target\n");

				exit(1);
			}

			setSnakePosition(t->x, t->y);

			self->targetY = self->y - 64;

			self->thinkTime = 60;

			self->action = &riseUp;

			playBossMusic();

			initBossHealthBar();
		}
	}
}

static void headWait()
{
	int x;

	/* Sway back and forth */

	self->dirX += 0.5;

	if (self->dirX >= 360)
	{
		self->dirX = 0;
	}

	x = 24;

	self->x = self->targetX + (sin(DEG_TO_RAD(self->dirX)) * x);

	alignBodyToHead();

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->thinkTime = 0;

		x = prand() % 4;

		switch (x)
		{
			case 0:
				self->action = &biteAttackInit;
			break;

			case 1:
				self->action = &changeSidesInit;
			break;

			case 2:
				self->action = &shotAttackInit;
			break;

			default:
				self->action = &crushAttackInit;
			break;
		}
	}
}

static void riseUp()
{
	Entity *e, *smoke;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->damage = 1;

		self->y -= 2;

		self->thinkTime = 0;

		self->flags &= ~NO_DRAW;

		self->takeDamage = &takeDamage;
	}

	alignBodyToHead();

	e = self->target;

	while (e != NULL)
	{
		if (e->target == NULL)
		{
			smoke = addSmoke(e->x + (prand() % self->w) * (prand() % 2 == 0 ? -1 : 1), e->y + prand() % e->h, "decoration/dust");

			if (smoke != NULL)
			{
				smoke->dirY = 0;
			}
		}

		e = e->target;
	}

	if (self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->targetX = self->x;

		self->dirX = 0;

		self->thinkTime = 120;

		self->action = &headWait;
	}
}

static void createBody()
{
	char bodyName[MAX_VALUE_LENGTH];
	int i;
	Entity **body, *head;

	body = (Entity **)malloc(bodyParts * sizeof(Entity *));

	if (body == NULL)
	{
		printf("Failed to allocate a whole %d bytes for snake body...\n", bodyParts * (int)sizeof(Entity *));

		exit(1);
	}

	snprintf(bodyName, sizeof(bodyName), "%s_body", self->name);

	/* Create in reverse order so that it is draw correctly */

	for (i=bodyParts-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			printf("No free slots to add a Snake Boss body part\n");

			exit(1);
		}

		loadProperties(bodyName, body[i]);

		body[i]->x = self->x;
		body[i]->y = self->y;

		body[i]->action = &bodyWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;
		body[i]->takeDamage = &bodyTakeDamage;

		body[i]->type = ENEMY;

		body[i]->active = FALSE;

		body[i]->flags |= NO_DRAW;

		setEntityAnimation(body[i], STAND);
	}

	/* Recreate the head so that it's on top */

	head = getFreeEntity();

	if (head == NULL)
	{
		printf("No free slots to add a Snake Boss head\n");
	}

	*head = *self;

	self->inUse = FALSE;

	self = head;

	headPart = head;

	/* Link the sections */

	for (i=bodyParts-1;i>=0;i--)
	{
		if (i == 0)
		{
			self->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];
		}
	}

	free(body);
}

static void biteAttackInit()
{
	facePlayer();

	setEntityAnimation(self, ATTACK_1);

	self->targetX = self->endX + (self->face == LEFT ? -32 : 32);
	self->targetY = self->endY - 32;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 4;
	self->dirY *= 4;

	self->action = &biteAttackWindUp;

	self->maxThinkTime = 1 + prand() % 3;
}

static void biteAttackWindUp()
{
	checkToMap(self);

	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->targetX = player.x;

		if (abs(self->x - self->targetX) > 320)
		{
			self->targetX = self->face == RIGHT ? self->x + 320 - self->w - 1 : self->x - 320;
		}

		self->action = &biteAttack;

		self->reactToBlock = &biteReactToBlock;
	}

	alignBodyToHead();
}

static void biteAttack()
{
	self->dirX = (self->targetX < self->x ? -self->speed * 2 : self->speed * 2);

	checkToMap(self);

	if (fabs(self->x - self->targetX) < self->speed)
	{
		self->x = self->targetX;

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &attackFinished;
		}

		else
		{
			self->targetX = self->endX + (self->face == LEFT ? -32 : 32);
			self->targetY = self->endY - 32;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 4;
			self->dirY *= 4;

			self->action = &biteAttackWindUp;
		}
	}

	alignBodyToHead();
}

static void shotAttackInit()
{
	facePlayer();

	setEntityAnimation(self, ATTACK_1);

	self->targetX = self->endX + (self->face == LEFT ? -50 : 50);
	self->targetY = self->endY - 32;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 4;
	self->dirY *= 4;

	self->action = &shotAttackWindUp;
	
	self->flags |= UNBLOCKABLE;
}

static void shotAttackWindUp()
{
	checkToMap(self);

	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->maxThinkTime = 5;

		self->thinkTime = 0;

		self->action = &shotAttack;
	}

	alignBodyToHead();
}

static void shotAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (prand() % 10 == 0)
		{
			e = addProjectile("boss/snake_boss_special_shot", self, self->x + self->w / 2, self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

			e->reactToBlock = &specialShotBlock;
		}

		else
		{
			e = addProjectile("boss/snake_boss_normal_shot", self, self->x + self->w / 2, self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

			e->reactToBlock = &bounceOffShield;
		}

		e->y -= e->h / 2;

		self->x += (self->face == LEFT ? 10 : -10);

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 10;
		}
	}

	alignBodyToHead();
}

static void setSnakePosition(int x, int y)
{
	Entity *e;

	self->x = x;
	self->y = y;

	self->endX = x;
	self->endY = y;

	e = self->target;

	while (e != NULL)
	{
		e->y = y;
		e->x = x + 23;

		e = e->target;
	}
}

static void changeSidesInit()
{
	self->targetX = self->endX;
	self->targetY = self->endY;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 2;
	self->dirY *= 2;

	self->action = &changeSides;
}

static void changeSides()
{
	Entity *e, *smoke;
	Target *t;

	self->x += self->dirX;
	self->y += self->dirY;

	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->flags |= NO_DRAW;

		self->takeDamage = NULL;

		self->targetX = self->x;

		self->dirX = 0;

		self->thinkTime = 120;

		t = getTargetByName(prand() % 2 == 0 ? "SNAKE_BOSS_TARGET_RIGHT" : "SNAKE_BOSS_TARGET_LEFT");

		if (t == NULL)
		{
			printf("Snake boss is missing target\n");

			exit(1);
		}

		self->damage = 0;

		setSnakePosition(t->x, t->y);

		self->targetX = self->x;
		self->targetY = self->y - 64;

		self->thinkTime = 120;

		self->action = &riseUpWait;
	}

	else
	{
		e = self->target;

		while (e != NULL)
		{
			if (e->target == NULL)
			{
				smoke = addSmoke(e->x + (prand() % self->w) * (prand() % 2 == 0 ? -1 : 1), e->y + prand() % e->h, "decoration/dust");

				if (smoke != NULL)
				{
					smoke->dirY = 0;
				}
			}

			e = e->target;
		}
	}

	alignBodyToHead();
}

static void crushAttackInit()
{
	self->maxThinkTime = 5;

	self->targetX = self->endX + (self->face == LEFT ? -75 : 75);
	self->targetY = self->endY - 128;

	self->action = &crushAttackMoveToPosition;
}

static void crushAttackMoveToPosition()
{
	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 6;
	self->dirY *= 6;

	checkToMap(self);

	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->y = self->targetY;

		self->x = self->targetX;

		self->dirY = 0;

		self->dirX = 0;

		self->action = &crushAttack;
	}

	alignBodyToHead();
}

static void crushAttack()
{
	self->dirX = 0;

	self->dirY = 8;

	self->flags |= UNBLOCKABLE;

	self->flags &= ~FLY;

	self->action = &crushAttackFinish;

	self->touch = &crushAttackHit;

	self->thinkTime = self->maxThinkTime == 1 ? 60 : 15;
}

static void crushAttackFinish()
{
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			addSmokeAlongBody();

			self->touch = &entityTouch;
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->maxThinkTime--;

			if (self->maxThinkTime <= 0)
			{
				self->action = &attackFinished;
			}

			else
			{
				self->targetX = self->targetX + (self->face == LEFT ? -75 : 75);
				self->targetY = self->endY - 128;

				self->action = &crushAttackMoveToPosition;
			}
		}
	}

	alignBodyToHead();
}

static void crushAttackHit(Entity *other)
{
	if (other->type == PLAYER)
	{
		self->thinkTime = 0;

		self->maxThinkTime = 0;
	}
}

static void riseUpWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 120;

		self->action = &riseUp;
	}
}

static void attackFinished()
{
	setEntityAnimation(self, STAND);

	self->flags &= ~UNBLOCKABLE;

	self->flags |= FLY;

	self->targetX = self->endX;
	self->targetY = self->endY - 64;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &returnToStart;
}

static void returnToStart()
{
	checkToMap(self);

	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->y = self->targetY;

		self->targetX = self->x;

		self->dirX = 0;

		self->thinkTime = 120;

		self->action = &headWait;
	}

	alignBodyToHead();
}

static void alignBodyToHead()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->x;
	y = self->y;

	partDistanceX = self->endX - self->x;
	partDistanceY = fabs(self->endY - self->y);

	partDistanceX /= bodyParts;
	partDistanceY /= bodyParts;

	e = self->target;

	while (e != NULL)
	{
		x += partDistanceX;
		y += partDistanceY;

		e->x = (e->target == NULL ? self->endX : x) + 23;
		e->y = (e->target == NULL ? self->endY : y);

		e->damage = self->damage;

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

static void takeDamage(Entity *other, int damage)
{
	int i;

	if (!(self->flags & INVULNERABLE))
	{
		/* Only takes proper damage against its own shot */

		if (strcmpignorecase(other->name, "boss/snake_boss_special_shot") == 0 && other->parent->type == PLAYER)
		{
			self->health -= damage;

			self->flags &= ~FLY;

			self->dirY = -2;

			self->dirX = self->face == LEFT ? -4 : 4;

			self->action = &stunned;
		}

		else
		{
			self->health -= (self->flags & HELPLESS ? damage * 3 : damage);
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			i = prand() % 3;

			switch (i)
			{
				case 0:
					playSoundToMap("sound/common/splat1.ogg", -1, self->x, self->y, 0);
				break;

				case 1:
					playSoundToMap("sound/common/splat2.ogg", -1, self->x, self->y, 0);
				break;

				default:
					playSoundToMap("sound/common/splat3.ogg", -1, self->x, self->y, 0);
				break;
			}
		}

		else
		{
			setEntityAnimation(self, PAIN);

			self->health = 0;

			self->thinkTime = 60;

			self->damage = 0;

			self->takeDamage = NULL;
			self->touch = NULL;

			self->targetX = self->endX;
			self->targetY = self->endY - 128;

			clearCustomActions(self);

			self->action = &die;

			self->dirX = self->face == RIGHT ? 2 : -2;

			self->flags &= ~(HELPLESS|FLY);
		}
	}
}

static void die()
{
	long onGround = self->flags & ON_GROUND;

	self->action = &die;

	checkToMap(self);

	if (self->flags & FLY)
	{
		if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
		{
			self->dirX = self->dirY = 0;

			self->thinkTime = 120;

			self->action = &fallToGround;
		}
	}

	else
	{
		if (self->flags & ON_GROUND)
		{
			self->dirX = 0;

			if (onGround == 0)
			{
				addSmokeAlongBody();
			}

			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 2;
				self->dirY *= 2;

				self->flags |= FLY;
			}
		}
	}

	alignBodyToHead();
}

static void fallToGround()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;

		self->dirX = self->face == LEFT ? -8 : 8;

		setEntityAnimation(self, DIE);
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, PAIN);

		self->thinkTime = 180;

		self->dirX = 0;

		self->action = &dieWait;

		addSmokeAlongBody();

		shakeScreen(MEDIUM, 90);

		e = self->target;

		while (e != NULL)
		{
			e->flags &= ~FLY;

			e = e->target;
		}

		fadeBossMusic();
	}

	else
	{
		alignBodyToHead();
	}
}

static void dieWait()
{
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		entityDieNoDrop();

		e = self;

		self = self->target;

		while (self != NULL)
		{
			self->die();

			self = self->target;
		}

		self = e;
	}
}

static void specialShotBlock()
{
	self->dirX *= -1;

	self->type = ENEMY;

	self->target = self->parent;

	self->parent = NULL;

	self->flags &= ~FLY;

	self->action = &specialShotWait;

	self->touch = NULL;

	self->thinkTime = 300;

	self->die = &entityDieNoDrop;
}

static void specialShotWait()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->touch = &specialShotTouch;

		self->dirX = 0;
	}

	if (self->thinkTime <= 0)
	{
		self->die();
	}
}

static void specialShotTouch(Entity *other)
{
	Entity *e;

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		e = addProjectile("boss/snake_boss_special_shot", &player, self->x, self->y, 0, 0);

		e->targetX = self->target->x + self->target->w / 2;
		e->targetY = self->target->y + self->target->h / 2;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 12;
		e->dirY *= 12;

		e->dirX = (player.face == RIGHT ? fabs(e->dirX) : fabs(e->dirX) * -1);

		e->damage = 10;

		self->inUse = FALSE;
	}
}

static void biteReactToBlock()
{
	self->maxThinkTime--;

	if (self->maxThinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	else
	{
		self->targetX = self->endX + (self->face == LEFT ? -32 : 32);
		self->targetY = self->endY - 32;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->action = &biteAttackWindUp;
	}
}

static void stunned()
{
	long onGround = self->flags & ON_GROUND;

	setEntityAnimation(self, PAIN);

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		setCustomAction(self, &helpless, 300, 0);

		if (onGround == 0)
		{
			addSmokeAlongBody();
		}

		self->action = &attackFinished;
	}

	alignBodyToHead();
}

static void addSmokeAlongBody()
{
	int i, bodyLength;

	shakeScreen(MEDIUM, 15);

	bodyLength = abs(self->endX - self->x);

	for (i=0;i<100;i++)
	{
		addSmoke((self->face == LEFT ? self->x : self->endX) + (prand() % bodyLength), self->y + prand() % self->h, "decoration/dust");
	}

	playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);
}

static void bodyTakeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	if (headPart->takeDamage != NULL)
	{
		/* Hitting the body only does half the damage */
	
		damage /= 2;
	
		temp = self;
	
		self = headPart;
		
		self->takeDamage(other, damage);
	
		self = temp;
	}
}
