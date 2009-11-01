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

#include "headers.h"

#include "graphics/animation.h"
#include "system/properties.h"
#include "map.h"
#include "geometry.h"
#include "inventory.h"
#include "collisions.h"
#include "custom_actions.h"
#include "collisions.h"
#include "player.h"
#include "entity.h"
#include "game.h"
#include "event/script.h"
#include "hud.h"
#include "audio/music.h"
#include "audio/audio.h"
#include "graphics/gib.h"
#include "projectile.h"
#include "graphics/decoration.h"
#include "system/error.h"

extern Entity player, playerShield, playerWeapon;
extern Entity *self;
extern Input input;
extern Game game;

static void takeDamage(Entity *, int);
static void attackFinish(void);
static void resetPlayer(void);
static void fallout(void);
static void slimeFallout(void);
static void falloutPause(void);
static void resetPause(void);
static void resetPlayer(void);
static void dialogWait(void);
static void playerDie(void);
static void alignAnimations(Entity *);
static void gameOverTimeOut(void);
static void touch(Entity *other);
static void applySlime(void);
static void swingSword(void);
static void drawBow(void);
static void fireArrow(void);
static int usingBow(void);

Entity *loadPlayer(int x, int y, char *name)
{
	loadProperties(name == NULL ? "edgar/edgar" : name, &player);

	if (player.inUse != TRUE)
	{
		player.inUse = TRUE;
		player.x = x;
		player.y = y;
		player.dirX = player.dirY = 0;
		player.face = LEFT;
		player.flags = 0;
		player.type = PLAYER;
		player.frameSpeed = 1;

		player.thinkTime = 0;

		player.maxHealth = player.health = 5;

		setEntityAnimation(&player, STAND);

		player.draw = &drawLoopingAnimationToMap;

		player.takeDamage = &takeDamage;

		playerShield.parent = &player;
		playerWeapon.parent = &player;

		playerWeapon.face = playerShield.face = LEFT;
	}

	else
	{
		player.flags = 0;

		playerWeapon.inUse = FALSE;
		playerShield.inUse = FALSE;

		loadInventoryItems();

		setEntityAnimation(&player, STAND);

		setPlayerLocation(x, y);

		centerMapOnEntity(&player);

		cameraSnapToTargetEntity();
	}

	clearCustomActions(&player);

	player.alpha = 255;

	player.element = NO_ELEMENT;

	player.weight = 1;

	player.originalWeight = player.weight;

	player.fallout = &fallout;

	player.die = &playerDie;

	player.action = NULL;

	player.touch = &touch;

	centerMapOnEntity(&player);

	return &player;
}

void setPlayerLocation(int x, int y)
{
	player.x = x;
	player.y = y;

	player.draw = &drawLoopingAnimationToMap;

	player.inUse = TRUE;
}

void setPlayerWeaponName(char *name)
{
	STRNCPY(playerWeapon.name, name, sizeof(playerWeapon.name));

	playerWeapon.inUse = TRUE;
}

void setPlayerShieldName(char *name)
{
	STRNCPY(playerShield.name, name, sizeof(playerShield.name));

	playerShield.inUse = TRUE;
}

void doPlayer()
{
	int i, j;

	self = &player;

	if (!(self->flags & TELEPORTING))
	{
		/* Gravity always pulls the player down */

		if (!(self->flags & FLY))
		{
			switch (self->environment)
			{
				case WATER:
				case SLIME:
				case LAVA:
					self->dirY += GRAVITY_SPEED * 0.25 * self->weight;

					if (self->dirY < -2)
					{
						self->dirY = -2;
					}

					else if (self->dirY >= MAX_WATER_SPEED)
					{
						self->dirY = MAX_WATER_SPEED;
					}
				break;

				default:
					self->dirY += GRAVITY_SPEED * self->weight;

					if (self->dirY >= MAX_AIR_SPEED)
					{
						self->dirY = MAX_AIR_SPEED;
					}

					else if (self->dirY > 0 && self->dirY < 1)
					{
						self->dirY = 1;
					}
				break;
			}
		}

		else
		{
			self->dirY = 0;
		}

		playerWeapon.frameSpeed = self->frameSpeed;
		playerShield.frameSpeed = self->frameSpeed;

		if (self->action == NULL)
		{
			self->flags &= ~(HELPLESS|INVULNERABLE|FLASH);

			for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
			{
				if (self->customAction[i].thinkTime > 0)
				{
					doCustomAction(&self->customAction[i]);
				}
			}

			if (!(self->flags & HELPLESS))
			{
				self->dirX = 0;

				if (!(self->flags & ON_GROUND))
				{
					self->flags &= ~GRABBING;

					if (self->target != NULL)
					{
						self->target->flags &= ~GRABBED;

						self->target = NULL;
					}
				}

				if (self->standingOn != NULL)
				{
					self->dirX = self->standingOn->dirX;

					if (self->standingOn->dirY > 0)
					{
						self->dirY = self->standingOn->dirY + 1;
					}
				}

				if (input.left == 1)
				{
					if (self->flags & BLOCKING)
					{
						playerWeapon.face = playerShield.face = self->face = LEFT;
					}

					else
					{
						if ((playerWeapon.flags & ATTACKING) && !(self->flags & ON_GROUND))
						{
							self->dirX -= self->speed;
						}

						else if (!(playerWeapon.flags & ATTACKING))
						{
							self->dirX -= self->speed;

							/* Only pull the target */

							if ((self->flags & GRABBING) && self->target != NULL)
							{
								self->target->dirX = -self->speed;

								self->target->frameSpeed = -1;
							}

							else
							{
								playerWeapon.face = playerShield.face = self->face = LEFT;
							}

							setEntityAnimation(self, WALK);
							setEntityAnimation(&playerShield, WALK);
							setEntityAnimation(&playerWeapon, WALK);
						}
					}
				}

				else if (input.right == 1)
				{
					if (self->flags & BLOCKING)
					{
						playerWeapon.face = playerShield.face = self->face = RIGHT;
					}

					else
					{
						if ((playerWeapon.flags & ATTACKING) && !(self->flags & ON_GROUND))
						{
							self->dirX += self->speed;
						}

						else if (!(playerWeapon.flags & ATTACKING))
						{
							self->dirX += self->speed;

							/* Only pull the target */

							if ((self->flags & GRABBING) && self->target != NULL)
							{
								self->target->dirX = self->speed;

								self->target->frameSpeed = 1;
							}

							else
							{
								playerWeapon.face = playerShield.face = self->face = RIGHT;
							}

							setEntityAnimation(self, WALK);
							setEntityAnimation(&playerShield, WALK);
							setEntityAnimation(&playerWeapon, WALK);
						}
					}
				}

				else if (input.left == 0 && input.right == 0 && !(self->flags & BLOCKING) && !(playerWeapon.flags & ATTACKING))
				{
					setEntityAnimation(self, STAND);
					setEntityAnimation(&playerShield, STAND);
					setEntityAnimation(&playerWeapon, STAND);

					if ((self->flags & GRABBING) && self->target != NULL)
					{
						self->target->dirX = 0;

						self->target->frameSpeed = 0;
					}
				}

				if (input.up == 1)
				{
					if (self->element == WATER && self->environment == WATER)
					{
						self->dirY = -self->speed;
					}

					else if (self->flags & FLY)
					{
						self->dirY = -self->speed;
					}

					else
					{
						if (self->standingOn != NULL)
						{
							if (self->standingOn->activate != NULL)
							{
								self = self->standingOn;

								self->activate(1);

								self = &player;
							}
						}

						input.up = 0;
					}
				}

				if (input.down == 1)
				{
					if (self->environment == WATER || (self->flags & FLY))
					{
						self->dirY = self->speed;
					}

					else
					{
						if (self->standingOn != NULL)
						{
							if (self->standingOn->activate != NULL)
							{
								self = self->standingOn;

								self->activate(-1);

								self = &player;

								self->dirY = self->standingOn->speed;
							}

							input.down = 0;
						}
					}
				}

				if (input.attack == 1 && !(self->flags & BLOCKING) && !(self->flags & GRABBED))
				{
					if (playerWeapon.inUse == TRUE && !(playerWeapon.flags & ATTACKING))
					{
						playerWeapon.flags |= ATTACKING;

						playerWeapon.action();
					}

					input.attack = 0;
				}

				if (input.interact == 1)
				{
					interactWithEntity(self->x, self->y, self->w, self->h);

					input.interact = 0;
				}

				if (input.grabbing == 1 && !(self->flags & BLOCKING))
				{
					self->flags |= GRABBING;
				}

				else
				{
					self->flags &= ~GRABBING;

					if (self->target != NULL)
					{
						self->target->flags &= ~GRABBED;

						self->target->frameSpeed = 0;

						self->target = NULL;
					}
				}

				if (input.block == 1 && (self->flags & ON_GROUND) && playerShield.inUse == TRUE && !(playerWeapon.flags & ATTACKING) && !(self->flags & GRABBED))
				{
					self->flags |= BLOCKING;

					setEntityAnimation(&playerWeapon, STAND);
					setEntityAnimation(self, BLOCK);
					setEntityAnimation(&playerShield, BLOCK);

					playerShield.thinkTime++;
				}

				else if ((input.block == 0 && (self->flags & BLOCKING)))
				{
					self->flags &= ~BLOCKING;

					setEntityAnimation(self, STAND);
					setEntityAnimation(&playerWeapon, STAND);
					setEntityAnimation(&playerShield, STAND);

					playerShield.thinkTime = 0;
				}

				if (input.activate == 1)
				{
					useInventoryItem();

					input.activate = 0;
				}

				if (input.next == 1 || input.previous == 1)
				{
					nextInventoryItem(input.next == 1 ? 1 : -1);

					input.next = input.previous = 0;
				}

				if (input.jump == 1 && !(self->flags & BLOCKING))
				{
					if (self->element == WATER && self->environment == WATER)
					{
						self->dirY = -self->speed;
					}

					else if (self->flags & ON_GROUND)
					{
						self->dirY = -JUMP_HEIGHT;
					}

					if (!(self->element == WATER && self->environment == WATER))
					{
						input.jump = 0;
					}
				}
				#if DEV == 1
					if (input.fly == 1)
					{
						self->flags ^= FLY;

						input.fly = 0;
					}
				#endif
			}

			else
			{
				self->flags &= ~GRABBING;

				if (self->target != NULL)
				{
					self->target->flags &= ~GRABBED;

					self->target = NULL;
				}

				if (self->standingOn != NULL)
				{
					self->dirX = self->standingOn->dirX;

					if (self->standingOn->dirY > 0)
					{
						self->dirY = self->standingOn->dirY + 1;
					}
				}
			}

			i = self->environment;

			if (!(self->flags & TELEPORTING))
			{
				checkToMap(self);

				if (self->environment == AIR && i == WATER && self->dirY < 0)
				{
					self->dirY = -JUMP_HEIGHT;
				}
			}

			self->standingOn = NULL;

			i = mapTileAt(self->x / TILE_SIZE, (self->y + self->h + 5) / TILE_SIZE);
			j = mapTileAt((self->x + self->w - 1) / TILE_SIZE, (self->y + self->h + 5) / TILE_SIZE);

			if ((self->flags & ON_GROUND) && i > BLANK_TILE && i < BACKGROUND_TILE_START && j > BLANK_TILE
				&& j < BACKGROUND_TILE_START && self->environment != WATER)
			{
				setCheckpoint(self->x, self->y);
			}
		}

		else
		{
			self->action();
		}

		addToGrid(self);

		if (playerWeapon.flags & ATTACKING)
		{
			addToGrid(&playerWeapon);
		}

		playerWeapon.alpha = player.alpha;
		playerShield.alpha = player.alpha;
	}

	else
	{
		doTeleport();
	}
}

void playerWaitForDialog()
{
	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerShield, STAND);
	setEntityAnimation(&playerWeapon, STAND);

	if (player.target != NULL)
	{
		player.face = player.x < player.target->x ? RIGHT : LEFT;

		playerWeapon.face = player.face;
		playerShield.face = player.face;
	}

	player.dirX = 0;

	player.action = &dialogWait;
}

void playerResumeNormal()
{
	player.target = NULL;

	player.action = NULL;
}

static void dialogWait()
{
	if (scriptWaiting() == TRUE)
	{
		readNextScriptLine();

		input.interact = 0;
		input.jump = 0;
		input.attack = 0;
		input.activate = 0;
		input.block = 0;
	}

	else if (input.interact == 1 || input.jump == 1 || input.attack == 1 || input.activate == 1 || input.block == 1)
	{
		readNextScriptLine();

		input.interact = 0;
		input.jump = 0;
		input.attack = 0;
		input.activate = 0;
		input.block = 0;
	}

	checkToMap(&player);

	if (player.target != NULL)
	{
		player.face = player.x < player.target->x ? RIGHT : LEFT;
	}
}

static void attackFinish()
{
	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerShield, STAND);
	setEntityAnimation(&playerWeapon, STAND);

	playerWeapon.x = playerShield.x = player.x;
	playerWeapon.y = playerShield.y = player.y;
}

void drawPlayer()
{
	int hasBow = usingBow();

	if (!(player.flags & NO_DRAW))
	{
		/* Draw the weapon if it's not a firing bow */

		self = &playerWeapon;

		if ((self->inUse == TRUE && hasBow == FALSE) || (hasBow == TRUE && !(self->flags & ATTACKING)))
		{
			if (!(player.flags & BLOCKING))
			{
				if (self->inUse == TRUE)
				{
					self->x = player.x;
					self->y = player.y;

					self->draw();
				}
			}
		}

		/* Draw the player */

		self = &player;

		self->draw();

		self = &playerWeapon;

		/* Draw the bow on top if it's being fired */

		if (hasBow == TRUE && (self->flags & ATTACKING))
		{
			self = &playerWeapon;

			self->x = player.x;
			self->y = player.y;

			self->draw();
		}

		else
		{
			/* Draw the shield */

			self = &playerShield;

			if (self->inUse == TRUE)
			{
				self->x = player.x;
				self->y = player.y;

				self->draw();
			}
		}
	}
	self = &player;

	if (self->health <= 0 && self->thinkTime == 0)
	{
		initGameOver();
	}
}

void setPlayerShield(int val)
{
	if (player.element == WATER)
	{
		setInventoryDialogMessage(_("Cannot equip items whilst transmogrified"));

		return;
	}

	if (usingBow() == TRUE)
	{
		/* Unequip the bow */

		playerWeapon.inUse = FALSE;
	}

	playerShield = *self;

	alignAnimations(&playerShield);

	if (game.status == IN_INVENTORY)
	{
		setInventoryDialogMessage(_("Equipped %s"), _(playerShield.objectiveName));
	}
}

void setPlayerWeapon(int val)
{
	if (player.element == WATER)
	{
		setInventoryDialogMessage(_("Cannot equip items whilst transmogrified"));

		return;
	}

	playerWeapon = *self;

	alignAnimations(&playerWeapon);

	/* Unset the shield */

	if (strcmpignorecase(playerWeapon.name, "weapon/bow") == 0)
	{
		playerWeapon.action = &drawBow;

		playerShield.inUse = FALSE;
	}

	else
	{
		playerWeapon.action = &swingSword;
	}

	if (game.status == IN_INVENTORY)
	{
		setInventoryDialogMessage(_("Equipped %s"), _(playerWeapon.objectiveName));
	}
}

void autoSetPlayerWeapon(Entity *newWeapon)
{
	if (player.element == WATER)
	{
		return;
	}

	if (strcmpignorecase(newWeapon->name, "weapon/normal_arrow") == 0)
	{
		return;
	}

	if (playerWeapon.inUse == FALSE)
	{
		/* Don't auto set the bow if the shield is in use */

		if (strcmpignorecase(newWeapon->name, "weapon/bow") == 0 && playerShield.inUse == TRUE)
		{
			return;
		}

		playerWeapon = *newWeapon;

		playerWeapon.action = (strcmpignorecase(playerWeapon.name, "weapon/bow") == 0 ? &drawBow : &swingSword);

		alignAnimations(&playerWeapon);
	}
}

void autoSetPlayerShield(Entity *newWeapon)
{
	if (player.element == WATER || usingBow() == TRUE)
	{
		return;
	}

	if (playerShield.inUse == FALSE)
	{
		playerShield = *newWeapon;

		alignAnimations(&playerShield);
	}
}

static void alignAnimations(Entity *e)
{
	e->parent = &player;

	e->face = player.face;

	e->inUse = TRUE;

	setEntityAnimation(e, getAnimationTypeAtIndex(&player));

	e->currentFrame = player.currentFrame;

	e->frameTimer = player.frameTimer;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (player.health <= 0 || other->parent == &playerWeapon)
	{
		return;
	}

	if ((player.flags & BLOCKING) && !(other->flags & UNBLOCKABLE))
	{
		if (other->type == PROJECTILE)
		{
			if ((other->dirX > 0 && player.face == LEFT) || (other->dirX < 0 && player.face == RIGHT))
			{
				if (other->element == NO_ELEMENT || (playerShield.element == other->element))
				{
					player.dirX = other->dirX < 0 ? -2 : 2;

					checkToMap(&player);

					setCustomAction(&player, &helpless, 2, 0);

					playSoundToMap("sound/edgar/block.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

					if (other->reactToBlock != NULL)
					{
						temp = self;

						self = other;

						self->reactToBlock();

						self = temp;

						return;
					}

					temp = self;

					self = other;

					self->die();

					self = temp;

					return;
				}
			}
		}

		else if ((other->dirX > 0 && player.face == LEFT) || (other->dirX < 0 && player.face == RIGHT)
			|| ((other->flags & ATTACKING) && ((other->x < player.x && player.face == LEFT) || (other->x > player.x && player.face == RIGHT))))
		{
			if (other->dirX != 0)
			{
				player.dirX = other->dirX < 0 ? -2 : 2;
			}

			else
			{
				player.dirX = player.x < other->x ? -6 : 6;
			}

			checkToMap(&player);

			setCustomAction(&player, &helpless, 2, 0);

			playSoundToMap("sound/edgar/block.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

			other->x = other->x < player.x ? player.x - other->w - 4 : player.x + player.w + 4;

			if (other->reactToBlock == NULL)
			{
				other->dirX = other->dirX < 0 ? 2 : -2;

				checkToMap(other);
			}

			else
			{
				temp = self;

				self = other;

				self->reactToBlock();

				self = temp;
			}

			return;
		}
	}

	if (!(player.flags & INVULNERABLE))
	{
		player.health -= damage;

		player.animationCallback = NULL;
		playerShield.animationCallback = NULL;
		playerWeapon.animationCallback = NULL;

		playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

		if (player.currentAnim != player.animation[DIE])
		{
			setEntityAnimation(&player, STAND);
			setEntityAnimation(&playerShield, STAND);
			setEntityAnimation(&playerWeapon, STAND);
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (player.health > 0)
		{
			setCustomAction(&player, &helpless, 10, 0);

			setCustomAction(&player, &invulnerable, 60, 0);

			if (player.dirX == 0)
			{
				if (other->dirX == 0)
				{
					player.dirX = other->x > player.x ? -6 : 6;
				}

				else
				{
					player.dirX = other->dirX < 0 ? -6 : 6;
				}
			}

			else
			{
				player.dirX = player.dirX < 0 ? 6 : -6;
			}
		}

		else
		{
			player.die();
		}
	}
}

int getDistanceFromPlayer(Entity *e)
{
	return getDistance(player.x, player.y, e->x, e->y);
}

void writePlayerMapStartToFile(FILE *fp)
{
	self = &player;

	fprintf(fp, "{\n");
	fprintf(fp, "TYPE PLAYER\n");
	fprintf(fp, "NAME PLAYER\n");
	fprintf(fp, "START_X %d\n", (int)self->x);
	fprintf(fp, "START_Y %d\n", (int)self->y);
	fprintf(fp, "}\n\n");
}

void writePlayerToFile(FILE *fp)
{
	self = &player;

	fprintf(fp, "{\n");
	fprintf(fp, "TYPE PLAYER\n");
	fprintf(fp, "NAME PLAYER\n");
	fprintf(fp, "START_X %d\n", (int)self->x);
	fprintf(fp, "START_Y %d\n", (int)self->y);
	fprintf(fp, "END_X %d\n", (int)self->endX);
	fprintf(fp, "END_Y %d\n", (int)self->endY);
	fprintf(fp, "THINKTIME %d\n", self->thinkTime);
	fprintf(fp, "HEALTH %d\n", self->health);
	fprintf(fp, "MAX_HEALTH %d\n", self->maxHealth);
	fprintf(fp, "DAMAGE %d\n", self->damage);
	/*fprintf(fp, "SPEED %0.2f\n", self->speed); Don't write the speed */
	/*fprintf(fp, "WEIGHT %0.2f\n", self->weight); Don't write the weight */
	fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
	fprintf(fp, "REQUIRES %s\n", self->requires);
	fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
	fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
	fprintf(fp, "}\n\n");

	self = &playerWeapon;

	if (self->inUse == TRUE)
	{
		fprintf(fp, "{\n");
		fprintf(fp, "TYPE PLAYER_WEAPON\n");
		fprintf(fp, "NAME %s\n", self->name);
		fprintf(fp, "START_X %d\n", (int)self->x);
		fprintf(fp, "START_Y %d\n", (int)self->y);
		fprintf(fp, "END_X %d\n", (int)self->endX);
		fprintf(fp, "END_Y %d\n", (int)self->endY);
		fprintf(fp, "THINKTIME %d\n", self->thinkTime);
		fprintf(fp, "HEALTH %d\n", self->health);
		fprintf(fp, "MAX_HEALTH %d\n", self->maxHealth);
		fprintf(fp, "DAMAGE %d\n", self->damage);
		/*fprintf(fp, "SPEED %0.2f\n", self->speed); Don't write the speed */
		fprintf(fp, "WEIGHT %0.2f\n", self->weight);
		fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
		fprintf(fp, "REQUIRES %s\n", self->requires);
		fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
		fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
		fprintf(fp, "}\n\n");
	}

	self = &playerShield;

	if (self->inUse == TRUE)
	{
		fprintf(fp, "{\n");
		fprintf(fp, "TYPE PLAYER_SHIELD\n");
		fprintf(fp, "NAME %s\n", self->name);
		fprintf(fp, "START_X %d\n", (int)self->x);
		fprintf(fp, "START_Y %d\n", (int)self->y);
		fprintf(fp, "END_X %d\n", (int)self->endX);
		fprintf(fp, "END_Y %d\n", (int)self->endY);
		fprintf(fp, "THINKTIME %d\n", self->thinkTime);
		fprintf(fp, "MAX_HEALTH %d\n", self->maxHealth);
		fprintf(fp, "HEALTH %d\n", self->health);
		fprintf(fp, "DAMAGE %d\n", self->damage);
		/*fprintf(fp, "SPEED %0.2f\n", self->speed); Don't write the speed */
		fprintf(fp, "WEIGHT %0.2f\n", self->weight);
		fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
		fprintf(fp, "REQUIRES %s\n", self->requires);
		fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
		fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
		fprintf(fp, "}\n\n");
	}
}

static void slimeFallout()
{
	if (player.environment != WATER)
	{
		fallout();
	}
}

static void fallout()
{
	if (!(player.flags & HELPLESS))
	{
		centerMapOnEntity(NULL);

		player.thinkTime = 120;

		player.dirX = 0;

		player.flags |= HELPLESS|INVULNERABLE;

		player.action = &falloutPause;

		player.touch = NULL;

		setEntityAnimation(&player, STAND);
		setEntityAnimation(&playerShield, STAND);
		setEntityAnimation(&playerWeapon, STAND);

		if (player.environment != AIR)
		{
			checkToMap(&player);
		}
	}
}

static void falloutPause()
{
	player.thinkTime--;

	if (player.thinkTime <= 0)
	{
		player.thinkTime = 60;

		player.action = &resetPause;
	}

	if (player.environment != AIR)
	{
		checkToMap(&player);
	}
}

static void resetPause()
{
	int minHealth;

	switch (player.environment)
	{
		case LAVA:
			minHealth = 99999;
		break;

		case SLIME:
			minHealth = 2;
		break;

		default:
			minHealth = 1;
		break;
	}

	if (player.health <= minHealth)
	{
		player.health = 0;

		if (player.environment != AIR)
		{
			checkToMap(&player);
		}

		player.die();
	}

	else if (player.health > 0)
	{
		player.thinkTime--;

		game.drawScreen = FALSE;

		if (player.thinkTime <= 0)
		{
			player.action = &resetPlayer;
		}

		if (player.environment != AIR)
		{
			checkToMap(&player);
		}
	}
}

static void resetPlayer()
{
	game.drawScreen = TRUE;

	player.draw = &drawLoopingAnimationToMap;

	getCheckpoint(&player.x, &player.y);

	centerMapOnEntity(&player);

	cameraSnapToTargetEntity();

	printf("Respawned at %f %f\n", player.x, player.y);

	player.action = NULL;

	player.touch = &touch;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	player.health -= (player.environment == SLIME ? 2 : 1);

	player.flags &= ~(HELPLESS|NO_DRAW);

	player.dirX = player.dirY = 0;

	player.y--;

	setCustomAction(&player, &invulnerable, 60, 0);
}

void increasePlayerMaxHealth()
{
	player.maxHealth++;

	player.health = player.maxHealth;

	setInfoBoxMessage(60, _("Maximum health has increased!"));
}

void syncWeaponShieldToPlayer()
{
	playerWeapon.face = playerShield.face = player.face;
	playerWeapon.face = playerShield.face = player.face;

	setEntityAnimation(&playerWeapon, getAnimationTypeAtIndex(&player));
	setEntityAnimation(&playerShield, getAnimationTypeAtIndex(&player));
}

static void playerDie()
{
	/* Change back to Edgar */

	if (player.element == WATER)
	{
		becomeEdgar();
	}

	setEntityAnimation(&player, DIE);
	setEntityAnimation(&playerShield, DIE);
	setEntityAnimation(&playerWeapon, DIE);

	player.frameSpeed = 1;

	player.health = 0;

	player.flags |= HELPLESS;

	playerWeapon.flags &= ~ATTACKING;

	player.dirX = 0;

	player.thinkTime = 180;

	player.action = &gameOverTimeOut;

	doGameOver();

	loadGameOverMusic();
}

static void gameOverTimeOut()
{
	player.thinkTime--;

	checkToMap(&player);
}

void freePlayer()
{
	player.inUse = FALSE;

	playerWeapon.inUse = FALSE;

	playerShield.inUse = FALSE;

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerWeapon, STAND);
	setEntityAnimation(&playerShield, STAND);
}

static void touch(Entity *other)
{

}

void playerGib()
{
	throwGibs("edgar/edgar_gibs", 6);

	player.inUse = TRUE;

	playerDie();

	player.flags |= NO_DRAW;
}

void facePlayer()
{
	self->face = player.x < self->x ? LEFT : RIGHT;
}

void setPlayerStunned(int thinkTime)
{
	/* Change back to Edgar */

	if (player.element == WATER)
	{
		becomeEdgar();
	}

	player.flags &= ~BLOCKING;

	setCustomAction(&player, &dizzy, thinkTime, 0);

	setEntityAnimation(&player, DIE);
	setEntityAnimation(&playerShield, DIE);
	setEntityAnimation(&playerWeapon, DIE);

	player.dirX = 0;
}

void doStunned()
{
	player.flags &= ~BLOCKING;

	setEntityAnimation(&player, DIE);
	setEntityAnimation(&playerShield, DIE);
	setEntityAnimation(&playerWeapon, DIE);
}

void setPlayerSlimed(int thinkTime)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Slimed Player");
	}

	/* Change back to Edgar */

	if (player.element == WATER)
	{
		becomeEdgar();
	}

	loadProperties("edgar/edgar_slimed", e);

	e->x = player.x;
	e->y = player.y;

	e->type = ENEMY;

	e->face = player.face;

	e->action = &applySlime;
	e->touch = touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = thinkTime;

	player.dirX = 0;

	setCustomAction(&player, &helpless, thinkTime, 0);

	player.flags &= ~BLOCKING;

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerWeapon, STAND);
	setEntityAnimation(&playerShield, STAND);
}

static void applySlime()
{
	self->thinkTime--;

	self->face = player.face;

	player.dirX = 0;

	self->x = player.x;
	self->y = player.y;

	if (self->thinkTime <= 0 || player.health <= 0)
	{
		self->inUse = FALSE;
	}
}

static void swingSword()
{
	setEntityAnimation(self, ATTACK_1);
	setEntityAnimation(&playerShield, ATTACK_1);
	setEntityAnimation(&playerWeapon, ATTACK_1);

	playSoundToMap("sound/edgar/swing.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

	playerWeapon.animationCallback = &attackFinish;
}

static void drawBow()
{
	if (!(player.flags & ON_GROUND))
	{
		fireArrow();
	}

	else
	{
		setEntityAnimation(&player, STAND);
		setEntityAnimation(&playerShield, ATTACK_1);
		setEntityAnimation(&playerWeapon, ATTACK_1);

		playerWeapon.animationCallback = &fireArrow;
	}
}

static void fireArrow()
{
	Entity *arrow, *e;

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerShield, ATTACK_1);
	setEntityAnimation(&playerWeapon, ATTACK_2);

	arrow = getInventoryItem(playerWeapon.requires);

	if (arrow != NULL)
	{
		e = addProjectile(arrow->name, &playerWeapon, playerWeapon.x + (player.face == RIGHT ? 0 : player.w), player.y + 15, player.face == RIGHT ? arrow->speed : -arrow->speed, 0);

		playSoundToMap("sound/edgar/arrow.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

		e->reactToBlock = &bounceOffShield;

		/*e->die = &stickToTarget;*/

		e->face = player.face;

		e->flags |= FLY;

		arrow->health--;

		if (arrow->health <= 0)
		{
			removeInventoryItem(playerWeapon.requires);
		}
	}

	else
	{
		printf("Couldn't get %s\n", playerWeapon.requires);
	}

	playerWeapon.animationCallback = &attackFinish;
}

void setBowAmmo(int val)
{
	Entity *bow = getInventoryItem("Bow");

	if (bow != NULL)
	{
		if (game.status == IN_INVENTORY)
		{
			setInventoryDialogMessage(_("Bow will now fire %s"), _(self->objectiveName));
		}

		else
		{
			setInfoBoxMessage(60, _("Bow will now fire %s"), _(self->objectiveName));
		}

		STRNCPY(bow->requires, self->objectiveName, sizeof(bow->requires));

		self = bow;

		setPlayerWeapon(0);
	}

	else
	{
		if (game.status == IN_INVENTORY)
		{
			setInventoryDialogMessage(_("A bow is required to use this item"));
		}

		else
		{
			setInfoBoxMessage(60, _("A bow is required to use this item"));
		}
	}
}

static int usingBow()
{
	if (playerWeapon.inUse == TRUE && strcmpignorecase(playerWeapon.name, "weapon/bow") == 0)
	{
		return TRUE;
	}

	return FALSE;
}

void becomeJumpingSlime(int seconds)
{
	int health, originalX, originalY, maxHealth;

	originalX = player.x;
	originalY = player.y;

	player.x = player.x + player.w / 2;
	player.y = player.y + player.h / 2;

	health = player.health;
	maxHealth = player.maxHealth;

	loadProperties("enemy/purple_jumping_slime", &player);

	player.x -= player.w / 2;
	player.y -= player.h / 2;

	/* If there wasn't enough space to transform then don't do anything */

	if (isValidOnMap(&player) == FALSE)
	{
		loadProperties("edgar/edgar", &player);

		player.x = originalX;
		player.y = originalY;

		setInfoBoxMessage(60, _("Cannot transmogrify here..."));
	}

	else
	{
		player.element = WATER;

		player.type = PLAYER;

		playSoundToMap("sound/common/teleport.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

		addParticleExplosion(player.x + player.w / 2, player.y + player.h / 2);

		playerWeapon.inUse = FALSE;
		playerShield.inUse = FALSE;

		player.fallout = &slimeFallout;

		setCustomAction(&player, &slimeTimeout, 60 * seconds, 0);
	}

	player.health = health;
	player.maxHealth = maxHealth;
}

void becomeEdgar()
{
	int bottom, midX;

	midX = player.x + player.w / 2;

	bottom = player.y + player.h;

	addParticleExplosion(player.x + player.w / 2, player.y + player.h / 2);

	loadProperties("edgar/edgar", &player);

	player.x = midX;

	player.x -= player.w / 2;

	player.element = NO_ELEMENT;

	player.environment = AIR;

	playSoundToMap("sound/common/teleport.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

	player.fallout = &fallout;

	clearCustomAction(&player, &slimeTimeout);

	player.y = bottom - player.h;

	if (player.health > 0)
	{
		if (strlen(playerWeapon.name) > 0)
		{
			playerWeapon.inUse = TRUE;
		}

		if (strlen(playerShield.name) > 0)
		{
			playerShield.inUse = TRUE;
		}
	}
}
