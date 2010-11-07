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
#include "../graphics/graphics.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../map.h"
#include "../item/item.h"
#include "../game.h"
#include "../audio/music.h"
#include "../audio/audio.h"
#include "../graphics/gib.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../geometry.h"
#include "../world/target.h"
#include "../hud.h"
#include "../player.h"
#include "../graphics/decoration.h"
#include "../system/error.h"

extern Entity *self, player;

static void init(void);
static void stand(void);
static void stand2(void);
static void dungeonTeleportInit(void);
static void offscreenTeleportInit(void);
static void dungeonTeleportWait(void);
static void teleportPlayerToDungeon(Entity *);
static void spellMove(void);
static void teleportOffscreenWait(void);
static void teleportPlayerOffscreen(Entity *);
static void arrowTakeDamage(Entity *, int);
static void disintegrationInit(void);
static void disintegrationWait(void);
static void disintegrationAttack(void);
static int drawSpell(void);
static void drawDisintegrationLine(int, int, int, int, int, int, int);
static void disintegrationWait(void);
static void disintegration(void);
static void distintegrationTouch(Entity *);

Entity *addSorceror(int x, int y, char *name)
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

	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case -5:
			self->action = &stand;
		break;
		
		case -4:
			self->action = &stand2;
			
			self->touch = &entityTouch;
			
			self->takeDamage = &arrowTakeDamage;
		break;
		
		default:
			self->action = &stand;
		break;
	}
}

static void stand()
{
	if (self->mental == -4)
	{
		setEntityAnimation(self, ATTACK_1);
		
		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		self->action = &dungeonTeleportInit;
	}
	
	checkToMap(self);
}

static void stand2()
{
	if (self->mental == -3)
	{
		setEntityAnimation(self, ATTACK_1);
		
		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		self->action = &offscreenTeleportInit;
	}
	
	checkToMap(self);
}

static void dungeonTeleportInit()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror Spell");
	}

	loadProperties("boss/sorceror_teleport_spell", e);
	
	setEntityAnimation(e, STAND);

	e->x = self->x + e->offsetX;
	e->y = self->y + e->offsetY;

	e->action = &spellMove;
	
	calculatePath(e->x, e->y, player.x + player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);
	
	e->dirX *= e->speed;
	e->dirY *= e->speed;

	e->draw = &drawLoopingAnimationToMap;
	
	e->touch = &teleportPlayerToDungeon;
	
	self->thinkTime = 60;
	
	self->action = &dungeonTeleportWait;
}

static void dungeonTeleportWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, STAND);
	}
	
	checkToMap(self);
}

static void spellMove()
{
	int i;
	Entity *e;
	
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
	
	else
	{
		for (i=0;i<2;i++)
		{
			e = addBasicDecoration(self->x, self->y, "decoration/particle");

			if (e != NULL)
			{
				e->x += prand() % self->w;
				e->y += prand() % self->h;

				e->thinkTime = 5 + prand() % 30;

				setEntityAnimation(e, prand() % 5);
			}
		}	
	}
}

static void teleportPlayerToDungeon(Entity *other)
{
	player.flags |= NO_DRAW;
	
	self->inUse = FALSE;
	
	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/teleport.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
}

static void offscreenTeleportInit()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror Spell");
	}

	loadProperties("boss/sorceror_teleport_spell", e);
	
	setEntityAnimation(e, STAND);

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

	e->action = &spellMove;
	
	calculatePath(e->x, e->y, player.x + player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);
	
	e->dirX *= e->speed;
	e->dirY *= e->speed;

	e->draw = &drawLoopingAnimationToMap;
	
	e->touch = &teleportPlayerOffscreen;
	
	self->thinkTime = 60;
	
	self->action = &teleportOffscreenWait;
}

static void teleportOffscreenWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
		
		setEntityAnimation(self, STAND);
		
		if (self->mental == -2)
		{
			self->action = &disintegrationInit;
		}
	}
}

static void disintegrationInit()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Disintegration Spell");
	}
	
	setEntityAnimation(self, ATTACK_2);
	
	loadProperties("boss/sorceror_disintegration_spell", e);
	
	setEntityAnimation(e, STAND);

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

	e->action = &disintegration;
	
	e->thinkTime = 120;
	
	e->startX = e->x;
	e->startY = e->y;
	
	e->head = self;
	
	e->endX = player.x + player.w / 2;
	e->endY = player.y + player.h / 2;

	e->draw = &drawLoopingAnimationToMap;
	
	self->action = &disintegrationWait;
}

static void disintegrationWait()
{
	checkToMap(self);
}

static void disintegration()
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
		playSoundToMap("sound/boss/sorceror/electrocute.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		self->x = self->endX;
		self->y = self->endY;
		
		self->thinkTime = 300;
		
		self->touch = &distintegrationTouch;
		
		self->action = &disintegrationAttack;
		
		self->draw = &drawSpell;
		
		self->layer = BACKGROUND_LAYER;
		
		self->flags &= ~NO_DRAW;
	}
}

static void distintegrationTouch(Entity *other)
{
	setEntityAnimation(&player, CUSTOM_1);
}

static int drawSpell()
{
	int color1, color2, color3;
	
	color1 = getColour(231, 231, 231);
	color2 = getColour(57, 57, 224);
	color3 = getColour(41, 41, 160);
	
	drawDisintegrationLine(self->startX, self->startY, self->endX, self->endY, color1, color2, color3);
	
	return TRUE;
}

static void disintegrationAttack()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->head->mental = -1;
		
		player.flags |= NO_DRAW;
		
		self->inUse = FALSE;
		
		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/teleport.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
	}
}

static void teleportPlayerOffscreen(Entity *other)
{
	Target *t;
	
	t = getTargetByName("SORCEROR_PLAYER_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Sorceror cannot find target");
	}

	player.targetX = t->x;
	player.targetY = t->y;

	calculatePath(player.x, player.y, player.targetX, player.targetY, &player.dirX, &player.dirY);

	player.flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	playSoundToMap("sound/common/spell.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
	
	self->inUse = FALSE;
}

static void arrowTakeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	if (!(self->flags & INVULNERABLE))
	{
		playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
		
		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock();

			self = temp;
		}

		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		
		damage = 0;
		
		addDamageScore(damage, self);
	}
}

static void drawDisintegrationLine(int x1, int y1, int x2, int y2, int color1, int color2, int color3)
{
	int speed;
	float dirX, dirY, x, y, ix, iy, yy;
	
	speed = 8;
	
	calculatePath(x1, y1, x2, y2, &dirX, &dirY);
	
	dirX *= speed;
	dirY *= speed;
	
	x = x1;
	y = y1;
	
	ix = x1;
	iy = y1;
	
	color1 = getColour(231, 231, 231);
	color2 = getColour(57, 57, 224);
	color3 = getColour(41, 41, 160);
	
	while (!(fabs(x2 - x) <= fabs(dirX)))
	{
		yy = (rand() % 8) * (rand() % 2 == 0 ? -1 : 1);
		
		drawColouredLine(x, y, x + dirX, y + dirY + yy, color1, color2, color3);
		
		x += dirX;
		y += dirY + yy;
		
		drawColouredLine(x, y, x + dirX, y + dirY - yy, color1, color2, color3);
		
		x += dirX;
		y += dirY - yy;
	}
}
