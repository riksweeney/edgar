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
#include "../graphics/decoration.h"
#include "../entity.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../player.h"
#include "../projectile.h"
#include "../map.h"
#include "../inventory.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../audio/audio.h"
#include "../game.h"
#include "../hud.h"

extern Entity *self;

static void jumpOverGap(void);
static void lookForPlayer(void);
static void moveAndJump(void);
static void jumpUp(void);
static int canJumpUp(void);
static int canDropDown(void);
static int isGapJumpable(void);
static void takeDamage(Entity *, int);
static void changeWall(void);
static void boxWait(void);
static void die(void);

Entity *addWallChanger(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Wall Changer");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		changeWall();
	}

	setEntityAnimation(self, "WALK");

	moveAndJump();
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

			setEntityAnimation(self, "STAND");
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

	if (onGround == 0 && ((self->flags & ON_GROUND) || (self->standingOn != NULL)))
	{
		self->action = &lookForPlayer;
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

	if (onGround == 0 && ((self->flags & ON_GROUND) || (self->standingOn != NULL)))
	{
		self->action = &lookForPlayer;
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
	int tile, i, j;
	int x = self->face == LEFT ? floor(self->x) : ceil(self->x) + self->w;
	int y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	for(j=0;j<3;j++)
	{
		for (i=0;i<8;i++)
		{
			tile = mapTileAt(x + (self->face == LEFT ? -j : j), y + i);

			if (tile >= WATER_TILE_START)
			{
				break;
			}

			if (tile != BLANK_TILE && tile < BACKGROUND_TILE_START)
			{
				return TRUE;
			}
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

static void die()
{
	Entity *e;
	
	if (prand() % 3 != 0)
	{
		e = addKeyItem("item/poison_meat", self->x + self->w / 2, self->y);

		e->x -= e->w / 2;

		e->action = &generalItemAction;

		e->flags |= DO_NOT_PERSIST;
	}

	entityDie();
}

static void changeWall()
{
	int r;
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Weapon Box");
	}

	loadProperties("boss/grimlore_weapon_box", e);
	
	r = hasLightningSword() == TRUE ? 6 : 5;

	switch (prand() % r)
	{
		case 0:
			STRNCPY(self->requires, "weapon/pickaxe", sizeof(self->requires));
			setEntityAnimationByID(e, 0);
		break;

		case 1:
			STRNCPY(self->requires, "weapon/wood_axe", sizeof(self->requires));
			setEntityAnimationByID(e, 1);
		break;

		case 2:
			STRNCPY(self->requires, "weapon/basic_sword", sizeof(self->requires));
			setEntityAnimationByID(e, 2);
		break;

		case 3:
			STRNCPY(self->requires, "weapon/normal_arrow", sizeof(self->requires));
			setEntityAnimationByID(e, 3);
		break;
		
		case 4:
			STRNCPY(self->requires, "weapon/flaming_arrow", sizeof(self->requires));
			setEntityAnimationByID(e, 4);
		break;
		
		default:
			STRNCPY(self->requires, "weapon/lighting_sword", sizeof(self->requires));
			setEntityAnimationByID(e, 5);
		break;
	}

	e->x = self->x + self->w / 2 - e-> w /2;
	e->y = self->y - e->h - 16;

	e->action = &boxWait;

	e->thinkTime = 60;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	self->thinkTime = 600;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (strcmpignorecase(other->name, self->requires) != 0)
	{
		playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

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

		damage = 0;

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}

	else
	{
		/* Always take 1 hit point damage */

		entityTakeDamageNoFlinch(other, 1);
	}
}

static void boxWait()
{
	self->x = self->head->x + self->head->w / 2 - self-> w / 2;
	self->y = self->head->y - self->h - 16;

	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		self->inUse = FALSE;
	}
}
