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
#include "../entity.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../player.h"
#include "../projectile.h"
#include "../map.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../audio/audio.h"

extern Entity *self, player, playerShield;

static void jumpOverGap(void);
static void lookForPlayer(void);
static void redLookForPlayer(void);
static void moveAndJump(void);
static void webAttack(void);
static void pounceAttack(void);
static void webAttackFinished(void);
static void webTouch(Entity *);
static void jumpUp(void);
static int canJumpUp(void);
static int canDropDown(void);
static int isGapJumpable(void);
static void pounceReactToBlock(void);
static void takeDamage(Entity *, int);
static void panic(void);

Entity *addLargeSpider(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Large Spider");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = strcmpignorecase(name, "enemy/large_red_spider") == 0 ? &redLookForPlayer : &lookForPlayer;
	e->resumeNormalFunction = strcmpignorecase(name, "enemy/large_red_spider") == 0 ? &redLookForPlayer : &lookForPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void redLookForPlayer()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	setEntityAnimation(self, WALK);

	moveAndJump();

	if (player.health > 0 && (prand() % 10 == 0) && self->thinkTime <= 0)
	{
		if (collision(self->x + (self->face == LEFT ? -160 : self->w), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			setEntityAnimation(self, STAND);
			
			self->thinkTime = 30;

			self->dirX = 0;
			
			self->mental = 3;

			self->action = &webAttack;

			facePlayer();
		}
	}
}

static void lookForPlayer()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	setEntityAnimation(self, WALK);

	moveAndJump();

	if (player.health > 0 && (prand() % 10 == 0) && self->thinkTime <= 0)
	{
		if (collision(self->x + (self->face == LEFT ? -64 : self->w), self->y, 64, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			setEntityAnimation(self, STAND);

			self->thinkTime = 30;

			self->dirX = 0;

			self->action = &pounceAttack;

			self->reactToBlock = &pounceReactToBlock;

			facePlayer();
		}
	}
}

static void moveAndJump()
{
	if (self->dirX == 0)
	{
		self->x += self->face == LEFT ? self->box.x : -self->box.x;

		self->face = self->face == RIGHT ? LEFT : RIGHT;
	}

	if (self->standingOn == NULL || self->standingOn->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	else
	{
		self->dirX += (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (isAtEdge(self) == TRUE)
	{
		if (isGapJumpable() == TRUE)
		{
			self->action = &jumpOverGap;

			setEntityAnimation(self, STAND);
		}

		else if (canDropDown() == FALSE)
		{
			self->dirX = 0;
		}
	}

	checkToMap(self);

	if (self->dirX == 0)
	{
		if (canJumpUp() == TRUE)
		{
			self->action = &jumpUp;
		}

		else
		{
			self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

			self->face = (self->face == RIGHT ? LEFT : RIGHT);
		}
	}
}

static void jumpUp()
{
	long onGround;

	if (self->flags & ON_GROUND)
	{
		self->dirY = -JUMP_HEIGHT;
	}

	self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

	onGround = (self->flags & ON_GROUND);

	checkToMap(self);

	if (onGround == 0 && (self->flags & ON_GROUND))
	{
		self->action = self->resumeNormalFunction;
	}
}

static void jumpOverGap()
{
	long onGround;

	self->dirX = (self->face == RIGHT ? 4 : -4);

	if (self->flags & ON_GROUND)
	{
		self->dirY = -JUMP_HEIGHT;
	}

	onGround = (self->flags & ON_GROUND);

	checkToMap(self);

	if (onGround == 0 && (self->flags & ON_GROUND))
	{
		self->action = self->resumeNormalFunction;
	}
}

static void pounceAttack()
{
	long onGround;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		checkToMap(self);
	}

	else
	{
		if (self->flags & ON_GROUND)
		{
			self->dirX = (self->face == RIGHT ? 6 : -6);

			self->dirY = -6;
		}

		onGround = (self->flags & ON_GROUND);

		checkToMap(self);

		if (onGround == 0 && (self->flags & ON_GROUND))
		{
			if (player.health > 0 && collision(self->x + (self->face == LEFT ? -16 : self->w), self->y, 16, self->h, player.x, player.y, player.w, player.h) == 1)
			{
				self->thinkTime = 60;

				self->dirX = 0;

				self->action = &pounceAttack;

				facePlayer();
			}

			else
			{
				self->action = self->resumeNormalFunction;

				self->reactToBlock = &changeDirection;

				self->dirX = self->face == LEFT ? -self->speed : self->speed;
			}
		}
	}
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		entityTakeDamageNoFlinch(other, damage);

		/* Jump away in panic */

		if ((prand() % 3 == 0) && self->face == other->face && self->health > 0)
		{
			self->dirX = self->x < player.x ? -3 : 3;

			self->dirY = -5;

			self->face = self->face == RIGHT ? LEFT : RIGHT;

			self->action = &panic;
		}
	}
}

static void panic()
{
	long onGround;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->action = self->resumeNormalFunction;
	}
}

static void webAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		e = addProjectile("misc/web", self, self->x, self->y + self->h / 2, (self->face == LEFT ? -8 : 8), 0);
		
		switch (self->mental)
		{
			case 3:
				e->dirY = -0.1;
			break;
			
			case 2:
				e->dirY = 0.1;
			break;
			
			default:
				e->dirY = 0;
			break;
		}

		e->x += self->face == LEFT ? -e->w : self->w;
		
		e->y -= e->h / 2;

		e->flags |= FLY;

		e->draw = &drawLoopingAnimationToMap;
		e->touch = &webTouch;

		e->face = self->face;

		setEntityAnimation(e, STAND);
		
		self->mental--;
		
		self->thinkTime = 15;

		self->action = self->mental == 0 ? &webAttackFinished : &webAttack;
	}
}

static void webAttackFinished()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = self->resumeNormalFunction;

		self->thinkTime = 180;
	}
}

static void webTouch(Entity *other)
{
	if (other->type == PLAYER && !(other->flags & WRAPPED) && !(other->flags & INVULNERABLE) && other->health > 0)
	{
		if ((other->flags & BLOCKING) && ((self->dirX > 0 && player.face == LEFT) || (self->dirX < 0 && player.face == RIGHT)))
		{
			player.dirX = self->dirX < 0 ? -2 : 2;

			checkToMap(&player);

			setCustomAction(&player, &helpless, 2, 0, 0);

			if (playerShield.thinkTime <= 0)
			{
				playSoundToMap("sound/edgar/block.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

				playerShield.thinkTime = 5;
			}
		}
		
		else
		{
			setPlayerWrapped(120);
		}

		self->inUse = FALSE;
	}
}

static int canJumpUp()
{
	int tile, tile2, i;
	int x = self->face == LEFT ? floor(self->x) : ceil(self->x) + self->w;
	int y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	x += self->face == LEFT ? -1 : 0;

	for (i=0;i<4;i++)
	{
		tile = mapTileAt(x, y - (i + 1));

		tile2 = mapTileAt(x, y - i);

		if (!(tile != BLANK_TILE && tile < BACKGROUND_TILE_START) && (tile2 != BLANK_TILE && tile2 < BACKGROUND_TILE_START))
		{
			return TRUE;
		}
	}

	return FALSE;
}

static int canDropDown()
{
	int tile, i;
	int x = self->face == LEFT ? floor(self->x) : ceil(self->x) + self->w;
	int y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	for (i=0;i<8;i++)
	{
		tile = mapTileAt(x, y + i);

		if (tile >= WATER_TILE_START)
		{
			return FALSE;
		}

		if (tile != BLANK_TILE && tile < BACKGROUND_TILE_START)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static int isGapJumpable()
{
	int tile;
	int x = self->face == LEFT ? floor(self->x) : ceil(self->x) + self->w;
	int y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	x += self->face == LEFT ? -3 : 3;

	tile = mapTileAt(x, y);

	if (tile != BLANK_TILE && tile < BACKGROUND_TILE_START)
	{
		return TRUE;
	}

	return FALSE;
}

static void pounceReactToBlock()
{
	self->dirX = self->x < player.x ? -3 : 3;

	self->dirY = -5;

	self->face = self->face == RIGHT ? LEFT : RIGHT;
}
