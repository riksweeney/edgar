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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void jumpOverGap(void);
static void lookForPlayer(void);
static void moveAndJump(void);
static void jumpUp(void);
static int canJumpUp(void);
static int canDropDown(void);
static int isGapJumpable(void);
static void takeDamage(Entity *, int);
static void changeArmour(void);
static void boxWait(void);
static void die(void);
static void init(void);
static int draw(void);
static void creditsMove(void);

Entity *addArmourChanger(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Armour Changer");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &draw;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;
	e->touch = &entityTouch;

	e->creditsAction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND_1");

	return e;
}

static void init()
{
	if (self->mental == 0)
	{
		self->mental = 1 + prand() % 3;
	}

	self->action = &lookForPlayer;

	self->creditsAction = &creditsMove;
}

static void lookForPlayer()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		changeArmour();
	}

	switch (self->mental)
	{
		case 2:
			setEntityAnimation(self, "WALK_2");
		break;

		case 3:
			setEntityAnimation(self, "WALK_3");
		break;

		default:
			setEntityAnimation(self, "WALK_1");
		break;
	}

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

			switch (self->mental)
			{
				case 2:
					setEntityAnimation(self, "STAND_2");
				break;

				case 3:
					setEntityAnimation(self, "STAND_3");
				break;

				default:
					setEntityAnimation(self, "STAND_1");
				break;
			}
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
	int tile, i, j, width;
	int x = self->face == LEFT ? floor(self->x) : ceil(self->x) + self->w;
	int y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	width = self->w / TILE_SIZE;

	for (j=0;j<width;j++)
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
				x = (x + (self->face == LEFT ? -j : j)) * TILE_SIZE;
				y = (y + i) * TILE_SIZE;

				return TRUE;
			}
		}
	}

	return FALSE;
}

static int isGapJumpable()
{
	int tile1, tile2;
	int x = self->face == LEFT ? floor(self->x) : ceil(self->x) + self->w;
	int y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	x += self->face == LEFT ? -3 : 3;

	tile1 = mapTileAt(x, y);

	if (tile1 != BLANK_TILE && tile1 < BACKGROUND_TILE_START)
	{
		y--;

		tile2 = mapTileAt(x, y);

		if (tile2 == BLANK_TILE || (tile2 >= BACKGROUND_TILE_START && tile2 <= BACKGROUND_TILE_END))
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void die()
{
	Entity *e;

	playSoundToMap("sound/enemy/armadillo/armadillo_die", -1, self->x, self->y, 0);

	if (prand() % 3 != 0)
	{
		e = addKeyItem("item/poison_meat", self->x + self->w / 2, self->y);

		e->x -= e->w / 2;

		e->action = &generalItemAction;

		e->flags |= DO_NOT_PERSIST;
	}

	entityDie();
}

static void changeArmour()
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
			STRNCPY(self->requires, "weapon/lightning_sword", sizeof(self->requires));
			setEntityAnimationByID(e, 5);
		break;
	}

	e->x = self->x + self->w / 2 - e-> w /2;
	e->y = self->y - e->h - 16;

	e->action = &boxWait;

	e->creditsAction = &boxWait;

	e->thinkTime = 60;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	self->thinkTime = 600;

	self->targetX = 60;

	self->targetY = self->targetX;
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

static int draw()
{
	int currentFrame, drawn;
	float frameTimer;
	char animationName[MAX_VALUE_LENGTH];

	drawn = drawLoopingAnimationToMap();

	if (self->targetX > 0)
	{
		self->targetX--;

		STRNCPY(animationName, self->animationName, MAX_VALUE_LENGTH);

		currentFrame = self->currentFrame;

		frameTimer = self->frameTimer;

		setEntityAnimation(self, strstr(animationName, "STAND") != NULL ? "GLOW_STAND" : "GLOW_WALK");

		self->currentFrame = currentFrame;

		self->alpha = (255 * self->targetX) / self->targetY;

		drawSpriteToMap();

		self->alpha = 255;

		setEntityAnimation(self, animationName);

		self->currentFrame = currentFrame;

		self->frameTimer = frameTimer;
	}

	return drawn;
}

static void creditsMove()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		changeArmour();

		self->thinkTime = 120;
	}

	switch (self->mental)
	{
		case 2:
			setEntityAnimation(self, "WALK_2");
		break;

		case 3:
			setEntityAnimation(self, "WALK_3");
		break;

		default:
			setEntityAnimation(self, "WALK_1");
		break;
	}

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
