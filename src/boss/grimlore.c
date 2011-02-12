/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../medal.h"
#include "../enemy/enemies.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../game.h"
#include "../player.h"
#include "../geometry.h"
#include "../graphics/decoration.h"
#include "../world/target.h"
#include "../enemy/rock.h"
#include "../system/error.h"

extern Entity *self, player;

Entity *addGrimlore(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;
	e->die = &die;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);
}

static void swordStabInit()
{
	self->thinkTime = 30;
	
	self->maxThinkTime = 1;

	self->action = &swordStab;

	checkToMap(self);
}

static void swordStab()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Sword");
		}

		loadProperties("boss/grimlore_sword_stab", e);

		e->x = self->x;
		e->y = self->y + self->h;

		e->action = &swordMoveUnderPlayer;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &swordTakeDamage;
		e->die = &swordDie;

		e->type = ENEMY;

		e->active = FALSE;

		setEntityAnimation(e, "STAND");

		e->startY = e->y;

		e->y -= e->h;

		e->endY = e->y;

		e->mental = 3 + prand() % 5;

		e->dirX = e->speed * 1.5;

		e->head = self;

		self->action = &swordStabWait;
	}

	checkToMap(self);
}

static void swordStabWait()
{
	if (self->maxThinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void swordMoveUnderPlayer()
{
	float target = player.x - self->w / 2 + player.w / 2;

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 30;

		self->action = &swordRise;
	}

	else
	{
		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &swordRise;
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &swordRise;
		}
	}
}

static void swordRise()
{
	Entity *e;

	if (self->y > self->startY)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->thinkTime == 0)
			{
				playSoundToMap("sound/common/crumble.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

			self->y -= 12;

			if (self->y <= self->startY)
			{
				self->y = self->startY;

				self->thinkTime = 30;
			}
		}
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime < 0)
		{
			self->action = &swordSink;
		}
	}

	facePlayer();
}

static void swordSink()
{
	if (self->y < self->targetY)
	{
		self->y += 3;
	}

	else
	{
		self->y = self->targetY;

		if (self->mental > 0 && player.health > 0)
		{
			self->action = &lookForPlayer;

			self->dirX = self->speed * 1.5;
		}

		else
		{
			self->action = &swordFinish;

			self->thinkTime = 60;
		}
	}
}

static void swordFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}
}

static void swordTakeDamage(Entity *other, int damage)
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

		addDamageScore(damage, self);
	}
}

static void swordDie()
{
	self->head->maxThinkTime = 0;

	self->inUse = FALSE;
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (self->head->inUse == TRUE)
		{
			/* The shield will take the damage instead */

			shieldTakeDamage(other, damage);

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}
		
		else if (self->mental > 0)
		{
			/* The armour will take the damage instead */

			armourTakeDamage(other, damage);

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}
		
		else
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

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			addDamageScore(damage, self);
		}
	}
}

static void armourTakeDamage(Entity *other, int damage)
{
	Entity *e, *temp;

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->health <= 0)
		{
			continue;
		}

		if (!(e->flags & INVULNERABLE))
		{
			e->health -= damage;

			if (e->health <= 0)
			{
				e->thinkTime = 180;

				e->flags &= ~FLY;

				e->dirY = ITEM_JUMP_HEIGHT;

				e->action = e->die;

				self->mental--;
			}

			else
			{
				setCustomAction(e, &flashWhite, 6, 0, 0);

				setCustomAction(e, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				enemyPain();
			}

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			addDamageScore(damage, self);
		}

		return;
	}
}

static void shieldWait()
{
	
}

static void shieldBiteInit()
{
	self->startX = self->x;
	
	self->thinkTime = 60;
	
	self->action = &shieldBiteWait;
}

static void shieldBiteWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == LEFT ? -self->speed * 3 : self->speed * 3;
		
		self->action = &shieldBite;
		
		self->reactToBlock = &shieldBiteReactToBlock;
	}
}

static void shieldBite()
{
	checkToMap(self);
	
	if (self->dirX == 0)
	{
		self->mental--;
		
		self->action = &shieldBiteReturn;
		
		self->dirX = self->face == LEFT ? self->speed * 3 : -self->speed * 3;
	}
}

static void shieldBiteReturn()
{
	if ((self->face == LEFT && self->x >= self->startX) || (self->face == RIGHT && self->x <= self->startX))
	{
		self->mental--;
		
		if (self->mental <= 0)
		{
			self->action = &shieldAttackFinish;
		}
		
		else
		{
			self->action = &shieldBiteWait;
		}
	}
}

static void shieldAttackFinish()
{
	self->head->maxThinkTime = 0;
	
	self->action = &shieldWait;
}

static void shieldRiftAttackInit()
{
	self->mental = 5 + prand() % 3;
	
	self->action = &shieldRiftAttack;
	
	self->thinkTime = 60;
}

static void shieldRiftAttack()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add an Energy Rift");
		}

		loadProperties("enemy/energy_rift", e);

		e->action = &riftMove;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

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
		
		e->damage = 1;

		e->y = self->y;

		e->dirX = self->face == LEFT ? -e->speed : e->speed;

		e->head = self;

		checkToMap(self);

		self->mental = 5 + prand() % 3;

		self->action = &riftAttackFinish;

		self->thinkTime = 60;
	}
	
	self->action = &shieldRiftWait;
	
	self->maxThinkTime = 0;
}

static void shieldRiftWait()
{
	if (self->maxThinkTime == 1)
	{
		
	}
}

static void riftMove()
{
	float dirY;

	self->dirX *= 0.95;

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
			self->head->maxThinkTime = 1;
			
			self->thinkTime = 60 + self->head->thinkTime * 60;

			self->action = &riftWait;
		}
	}

	dirY = self->dirY;

	checkToMap(self);
}

static void riftWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &riftClose;

		self->thinkTime = 20;
	}

	else
	{
		if (collision(self->x - self->mental, self->y - self->mental, self->mental * 2, self->mental * 2, player.x, player.y, player.w, player.h) == 1)
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
