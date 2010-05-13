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
#include "../game.h"
#include "../enemy/enemies.h"
#include "../projectile.h"
#include "../system/error.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void attackFinished(void);
static void wait(void);
static void introWait(void);
static void teleportAway(void);
static void teleportWait(void);
static void teleportIn(void);
static int energyBarDraw(void);
static void energyBarWait(void);
static void initEnergyBar(void);
static void takeDamage(Entity *, int);
static void die(void);
static void healPartner(void);
static void addStunStar(void);
static void starWait(void);

Entity *addAwesomeBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Awesome Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = &takeDamage;
	
	e->die = &die;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initEnergyBar()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Awesome Boss Energy Bar");
	}

	loadProperties("boss/awesome_boss_energy_bar", e);

	e->action = &energyBarWait;

	e->draw = &energyBarDraw;

	e->type = ENEMY;

	e->active = FALSE;
	
	e->head = self;

	setEntityAnimation(e, STAND);
	
	self->head = getEntityByObjectiveName("AWESOME_BOSS_METER");
	
	if (self->head == NULL)
	{
		showErrorAndExit("Awesome Boss could not find meter");
	}
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
		
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);
			
			self->thinkTime = 60;
			
			self->mental = 2;
			
			self->action = &doIntro;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	char name[MAX_VALUE_LENGTH];
	Entity *e;
	
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->flags |= DO_NOT_PERSIST;
		
		snprintf(name, sizeof(name), "boss/awesome_boss_%d", self->mental);
		
		e = addEnemy(name, self->x - 8 * self->mental, self->y - 64);
		
		e->face = RIGHT;
		
		e->active = TRUE;

		e->targetX = e->x;
		e->targetY = e->y;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);
		
		e->action = &introWait;
		
		e->head = self;

		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		self->thinkTime = 30;
		
		self->mental++;
		
		if (self->mental == 5)
		{
			self->target = getEntityByObjectiveName(self->requires);
			
			if (self->target == NULL)
			{
				showErrorAndExit("Awesome Boss %s cannot find %s", self->objectiveName, self->requires);
			}
			
			self->thinkTime = 60;
			
			self->action = &wait;
		}
	}
}

static void attackFinished()
{
	checkToMap(self);
	
	self->thinkTime = 30 + prand() % 30;
	
	self->action = &teleportAway;
}

static void teleportAway()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->touch = NULL;
		
		self->flags |= NO_DRAW;
		
		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
		
		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		self->action = &teleportWait;
		
		self->thinkTime = 60 + prand() % 120;
		
		/* Choose if they attack again, or their partner */
		
		if (self->active == TRUE)
		{
			if (prand() % 2 == 0)
			{
				self->target->thinkTime = 60 + prand() % 120;
				
				self->target->active = FALSE;
			}
			
			else
			{
				self->thinkTime = 60 + prand() % 120;
				
				self->active = FALSE;
				
				self->target->active = TRUE;
			}
		}
	}
	
	checkToMap(self);
}

static void teleportWait()
{
	Target *t;
	
	if (self->target->health <= 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			printf("Teleporting in to save partner\n");
			
			self->x = self->target->x;
			
			t = getTargetByName((prand() % 2 == 0) ? "AWESOME_TARGET_LEFT" : "AWESOME_TARGET_RIGHT");
			
			if (t == NULL)
			{
				showErrorAndExit("Awesome Boss cannot find target");
			}
			
			self->y = t->y;
			
			faceTarget();
			
			self->thinkTime = 60;
			
			self->flags &= ~NO_DRAW;
			
			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
			
			playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);
			
			self->action = &healPartner;
			
			self->touch = &entityTouch;
			
			self->dirY = 0;
		}
	}
	
	if (self->active == TRUE)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			t = getTargetByName((prand() % 2 == 0) ? "AWESOME_TARGET_LEFT" : "AWESOME_TARGET_RIGHT");
			
			if (t == NULL)
			{
				showErrorAndExit("Awesome Boss cannot find target");
			}
			
			self->x = t->x + (prand() % 16) * (prand() % 2 == 0 ? 1 : -1);
			self->y = t->y;
			
			facePlayer();
			
			self->thinkTime = 180;
			
			self->flags &= ~NO_DRAW;
			
			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
			
			playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);
			
			self->action = &teleportIn;
			
			self->touch = &entityTouch;
			
			setEntityAnimation(self, STAND);
			
			self->dirY = 0;
		}
	}
	
	checkToMap(self);
}

static void teleportIn()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}
	
	checkToMap(self);
}

static void introWait()
{
	if (self->head->mental == 5 && self->head->thinkTime == 0)
	{
		self->target = getEntityByObjectiveName(self->requires);
		
		if (self->target == NULL)
		{
			showErrorAndExit("Awesome Boss %s cannot find %s", self->objectiveName, self->requires);
		}
		
		self->action = &teleportAway;
		
		self->touch = &entityTouch;
		
		initEnergyBar();
	}
	
	checkToMap(self);
}

static void wait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime == 0)
		{
			playBossMusic();
			
			initEnergyBar();
			
			self->action = &teleportAway;
			
			self->touch = &entityTouch;
		}
	}
	
	checkToMap(self);
}

static void energyBarWait()
{
	self->x = self->head->x - (self->w - self->head->w) / 2;
	self->y = self->head->y - self->h - 4;
	
	if (self->health < self->head->health)
	{
		self->health += (self->head->health / 100);

		if (self->health > self->head->health)
		{
			self->health = self->head->health;
		}
	}

	else if (self->head->health < self->health)
	{
		self->health -= 3;

		if (self->health < self->head->health)
		{
			self->health = self->head->health;
		}
	}
}

static int energyBarDraw()
{
	int width;
	float percentage;
	
	if (!(self->head->flags & NO_DRAW))
	{
		drawLoopingAnimationToMap();
		
		percentage = self->health;
		percentage /= self->head->maxHealth;
		
		width = self->w - 2;
		
		width *= percentage;
		
		if (percentage >= 0.5)
		{
			drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 0, 220, 0);
		}
		
		else if (percentage >= 0.25)
		{
			drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 220, 220, 0);
		}
		
		else
		{
			drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 220, 0, 0);
		}
	}
	
	return TRUE;
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
		self->health -= damage;
		
		temp = self;
		
		self = self->head;
		
		self->takeDamage(other, damage);

		if (self->health <= 0)
		{
			self->touch = NULL;
			
			setEntityAnimation(self, DIE);
			
			addStunStar();
			
			self->action = self->die;
		}

		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

		setCustomAction(self, &flashWhite, 6, 0, 0);

		/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

		if (other->type != PROJECTILE)
		{
			setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		}

		if (self->pain != NULL)
		{
			self->pain();
		}
	}
}

static void die()
{
	if (self->target->health <= 0)
	{
		self->action = &entityDie;
	}
	
	checkToMap(self);
}

static void healPartner()
{
	if (self->flags & ON_GROUND)
	{
		self->target->health++;
		
		if (self->target->health >= self->target->maxHealth)
		{
			self->target->health = self->target->maxHealth;
			
			self->target->action = &teleportAway;
			
			self->action = &teleportAway;
		}
	}
	
	checkToMap(self);
}

static void addStunStar()
{
	int i;
	Entity *e;
	
	for (i=0;i<2;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Awesome Boss's Star");
		}

		loadProperties("boss/armour_boss_star", e);

		e->x = self->x;
		e->y = self->y;

		e->action = &starWait;

		e->draw = &drawLoopingAnimationToMap;

		e->thinkTime = 300;

		e->head = self;

		setEntityAnimation(e, STAND);

		e->currentFrame = (i == 0 ? 0 : 6);

		e->x = self->x + self->w / 2 - e->w / 2;

		e->y = self->y - e->h;
	}	
}

static void starWait()
{
	if (self->head->health == self->head->maxHealth || self->head->target->health <= 0)
	{
		self->inUse = FALSE;
	}
	
	self->x = self->head->x + self->head->w / 2- self->w / 2;

	self->y = self->head->y - self->h;
}
