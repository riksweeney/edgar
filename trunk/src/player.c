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

#include "headers.h"

#include "graphics/animation.h"
#include "system/properties.h"
#include "map.h"
#include "medal.h"
#include "geometry.h"
#include "inventory.h"
#include "collisions.h"
#include "custom_actions.h"
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
#include "system/random.h"
#include "item/item.h"

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
static void playerWait(void);
static void applyIce(void);
static void applyWebbing(void);
static void lightningSwordTouch(Entity *);
static void lightningSwordChangeToBasic(void);
static void weaponTouch(Entity *);
static void resurrectionTimeOut(void);
static void resurrectionParticleWait(void);
static void resurrectionWait(void);
static void confirmWait(void);

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

		#if DEV == 1
			player.maxHealth = player.health = 5;
		#endif

		setEntityAnimation(&player, "STAND");

		player.draw = &drawLoopingAnimationToMap;

		player.takeDamage = &takeDamage;

		playerShield.parent = &player;
		playerWeapon.parent = &player;

		playerShield.name[0] = '\0';
		playerWeapon.name[0] = '\0';

		playerWeapon.face = playerShield.face = LEFT;
	}

	else
	{
		player.flags = 0;

		playerWeapon.inUse = FALSE;
		playerShield.inUse = FALSE;

		loadInventoryItems();

		setEntityAnimation(&player, "STAND");

		setPlayerLocation(x, y);

		centerMapOnEntity(&player);

		cameraSnapToTargetEntity();
	}

	playerWeapon.flags = 0;
	playerShield.flags = 0;

	playerWeapon.animationCallback = NULL;
	playerShield.animationCallback = NULL;

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
	long travelled;

	travelled = game.distanceTravelled;

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
			self->flags &= ~(HELPLESS|INVULNERABLE|FLASH|ATTRACTED);

			for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
			{
				if (self->customAction[i].thinkTime > 0)
				{
					doCustomAction(&self->customAction[i]);
				}
			}

			if (!(self->flags & HELPLESS))
			{
				/* Don't reset the speed if being pulled towards target */

				if (!(self->flags & ATTRACTED))
				{
					self->dirX = 0;
				}

				if (!(self->flags & ON_GROUND) || (self->standingOn != NULL && self->standingOn->dirY != 0))
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

					self->flags |= ON_GROUND;
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

							if ((self->flags & GRABBING) && self->target != NULL && self->target->x > self->x)
							{
								self->target->dirX = -self->speed;

								self->target->frameSpeed = -1;
							}

							else
							{
								playerWeapon.face = playerShield.face = self->face = LEFT;
							}

							setEntityAnimation(self, "WALK");
							setEntityAnimation(&playerShield, "WALK");
							setEntityAnimation(&playerWeapon, "WALK");
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

							if ((self->flags & GRABBING) && self->target != NULL && self->target->x < self->x)
							{
								self->target->dirX = self->speed;

								self->target->frameSpeed = 1;
							}

							else
							{
								playerWeapon.face = playerShield.face = self->face = RIGHT;
							}

							setEntityAnimation(self, "WALK");
							setEntityAnimation(&playerShield, "WALK");
							setEntityAnimation(&playerWeapon, "WALK");
						}
					}
				}

				else if (input.left == 0 && input.right == 0 && !(self->flags & BLOCKING) && !(playerWeapon.flags & ATTACKING))
				{
					setEntityAnimation(self, "STAND");
					setEntityAnimation(&playerShield, "STAND");
					setEntityAnimation(&playerWeapon, "STAND");

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

				if (input.attack == 1 && !(self->flags & GRABBED))
				{
					if (!(self->flags & BLOCKING))
					{
						if (playerWeapon.inUse == TRUE && !(playerWeapon.flags & ATTACKING))
						{
							playerWeapon.flags |= ATTACKING;

							playerWeapon.action();
						}
					}

					else
					{
						if (playerShield.health >= 4)
						{
							printf("Will throw shield\n");
						}
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

					setEntityAnimation(&playerWeapon, "STAND");
					setEntityAnimation(self, "BLOCK");
					setEntityAnimation(&playerShield, "BLOCK");

					playerShield.mental++;
				}

				else if ((input.block == 0 && (self->flags & BLOCKING)))
				{
					self->flags &= ~BLOCKING;

					setEntityAnimation(self, "STAND");
					setEntityAnimation(&playerWeapon, "STAND");
					setEntityAnimation(&playerShield, "STAND");

					playerShield.mental = 0;
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
						if (self->element == WATER)
						{
							if (prand() % 3 == 0)
							{
								playSoundToMap("sound/enemy/jumping_slime/jump2.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
							}

							else
							{
								playSoundToMap("sound/enemy/jumping_slime/jump1.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
							}
						}

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

		playerShield.thinkTime--;

		if (playerShield.thinkTime <= 0)
		{
			playerShield.thinkTime = 0;
		}

		playerWeapon.thinkTime--;

		if (playerWeapon.thinkTime <= 0)
		{
			playerWeapon.thinkTime = 0;
		}

		game.distanceTravelled += fabs(self->dirX);

		if (travelled < 2250000 && game.distanceTravelled >= 2250000)
		{
			addMedal("50km");
		}
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

	setEntityAnimation(&player, "STAND");
	setEntityAnimation(&playerShield, "STAND");
	setEntityAnimation(&playerWeapon, "STAND");

	if (playerWeapon.mental == -1)
	{
		lightningSwordChangeToBasic();
	}

	if (player.target != NULL)
	{
		player.face = player.x < player.target->x ? RIGHT : LEFT;

		playerWeapon.face = player.face;
		playerShield.face = player.face;
	}

	player.dirX = 0;

	player.flags |= INVULNERABLE;

	player.action = &dialogWait;
}

void playerResumeNormal()
{
	if (player.health != 0)
	{
		player.target = NULL;

		player.action = NULL;

		player.flags &= ~INVULNERABLE;
	}
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

		playerShield.face = player.face;
		playerWeapon.face = player.face;
	}

	if (self->standingOn != NULL)
	{
		self->dirX = self->standingOn->dirX;

		if (self->standingOn->dirY > 0)
		{
			self->dirY = self->standingOn->dirY + 1;
		}

		self->flags |= ON_GROUND;
	}
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}
}

void playerWaitForConfirm()
{
	player.action = &confirmWait;
}

static void confirmWait()
{
	doScriptMenu();

	checkToMap(&player);

	if (player.target != NULL)
	{
		player.face = player.x < player.target->x ? RIGHT : LEFT;

		playerShield.face = player.face;
		playerWeapon.face = player.face;
	}
}

static void attackFinish()
{
	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	setEntityAnimation(&player, "STAND");
	setEntityAnimation(&playerShield, "STAND");
	setEntityAnimation(&playerWeapon, "STAND");

	if (playerWeapon.mental == -1)
	{
		lightningSwordChangeToBasic();
	}

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

	/* Don't allow the player to change shields whilst attacking */

	if (playerWeapon.flags & ATTACKING)
	{
		return;
	}

	if (usingBow() == TRUE)
	{
		playerWeapon.animationCallback = NULL;

		setEntityAnimation(&player, "STAND");

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

	/* Don't allow the player to change weapons whilst attacking */

	if (playerWeapon.flags & ATTACKING)
	{
		return;
	}

	playerWeapon.animationCallback = NULL;

	playerWeapon = *self;

	playerWeapon.head = self;

	playerWeapon.touch = &weaponTouch;

	alignAnimations(&playerWeapon);

	/* Unset the shield */

	if (strcmpignorecase(playerWeapon.name, "weapon/bow") == 0)
	{
		playerWeapon.action = &drawBow;

		playerShield.name[0] = '\0';

		playerShield.inUse = FALSE;
	}

	else if (strcmpignorecase(playerWeapon.name, "weapon/lightning_sword") == 0)
	{
		playerWeapon.action = &swingSword;

		if (playerWeapon.mental > 0)
		{
			playerWeapon.touch = &lightningSwordTouch;
		}
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

		playerWeapon.head = newWeapon;

		if (strcmpignorecase(playerWeapon.name, "weapon/bow") == 0)
		{
			playerWeapon.action = &drawBow;

			playerShield.inUse = FALSE;
		}

		else if (strcmpignorecase(playerWeapon.name, "weapon/lightning_sword") == 0)
		{
			playerWeapon.action = &swingSword;

			if (playerWeapon.mental > 0)
			{
				playerWeapon.touch = &lightningSwordTouch;
			}
		}

		else
		{
			playerWeapon.action = &swingSword;
		}

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
	int shieldHealth;
	Entity *temp;
	char shieldName[MAX_VALUE_LENGTH], oldShieldName[MAX_VALUE_LENGTH];

	if (player.health <= 0 || other->parent == &playerWeapon)
	{
		return;
	}

	if ((player.flags & BLOCKING) && !(other->flags & UNBLOCKABLE) && (other->element == NO_ELEMENT || (playerShield.element == other->element)))
	{
		if (other->type == PROJECTILE)
		{
			if ((other->dirX > 0 && player.face == LEFT) || (other->dirX < 0 && player.face == RIGHT))
			{
				player.dirX = other->dirX < 0 ? -2 : 2;

				checkToMap(&player);

				setCustomAction(&player, &helpless, 2, 0, 0);

				if (playerShield.thinkTime <= 0)
				{
					playSoundToMap("sound/edgar/block.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

					playerShield.thinkTime = 5;
				}

				game.attacksBlocked++;

				if (game.attacksBlocked == 2000)
				{
					addMedal("blocked");
				}

				if (other->reactToBlock != NULL)
				{
					temp = self;

					self = other;

					self->reactToBlock(temp);

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

			setCustomAction(&player, &helpless, 2, 0, 0);

			if (playerShield.thinkTime <= 0)
			{
				playSoundToMap("sound/edgar/block.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

				playerShield.thinkTime = 5;
				
				if (strcmpignorecase(other->objectiveName, "SORCEROR_DISINTEGRATION_SPELL") == 0)
				{
					if (strcmpignorecase(playerShield.objectiveName, "Disintegration Shield") == 0)
					{
						playerShield.health++;
						
						if (playerShield.health >= 4)
						{
							setInfoBoxMessage(240, 255, 255, 255, _("Press Attack whilst Blocking to throw your shield..."));
							
							playerShield.health = 4;
						}
						
						else
						{
							shieldHealth = playerShield.health;
							
							STRNCPY(oldShieldName, playerShield.name, sizeof(oldShieldName));
							
							snprintf(shieldName, MAX_VALUE_LENGTH, "weapon/disintegration_shield_%d", shieldHealth);
							
							loadProperties(shieldName, &playerShield);
							
							replaceInventoryItemWithName(oldShieldName, &playerShield);
							
							playerShield.thinkTime = 5;
							
							playerShield.health = shieldHealth;
							
							alignAnimations(&playerShield);
						}
					}
				}
			}

			game.attacksBlocked++;

			if (game.attacksBlocked == 2000)
			{
				addMedal("blocked");
			}

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

				self->reactToBlock(temp);

				self = temp;
			}

			return;
		}
	}

	if (!(player.flags & INVULNERABLE))
	{
		if (game.infiniteEnergy == FALSE)
		{
			player.health -= damage;
		}

		player.animationCallback = NULL;
		playerShield.animationCallback = NULL;
		playerWeapon.animationCallback = NULL;

		playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

		if (strcmpignorecase(player.animationName, "DIE") != 0)
		{
			setEntityAnimation(&player, "STAND");
			setEntityAnimation(&playerShield, "STAND");
			setEntityAnimation(&playerWeapon, "STAND");
		}

		if (playerWeapon.mental == -1)
		{
			lightningSwordChangeToBasic();
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
			if (self->action != NULL)
			{
				self->action = NULL;

				if (self->target != NULL)
				{
					self->target = NULL;
				}
			}

			setCustomAction(&player, &helpless, 10, 0, 0);

			setCustomAction(&player, &invulnerable, 60, 0, 0);

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
	fprintf(fp, "MENTAL %d\n", self->mental);
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
		fprintf(fp, "MENTAL %d\n", self->mental);
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
		fprintf(fp, "MENTAL %d\n", self->mental);
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

		setEntityAnimation(&player, "STAND");
		setEntityAnimation(&playerShield, "STAND");
		setEntityAnimation(&playerWeapon, "STAND");

		checkToMap(&player);
	}
}

static void falloutPause()
{
	player.thinkTime--;

	if (player.flags & NO_DRAW)
	{
		player.dirY = 0;

		player.flags |= FLY;
	}

	if (player.thinkTime <= 0)
	{
		player.thinkTime = 60;

		player.action = &resetPause;
	}

	checkToMap(&player);
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

	if ((player.health <= minHealth && getInventoryItemByObjectiveName("Amulet of Resurrection") == NULL) || player.environment == LAVA)
	{
		player.health = 0;

		if (player.environment != AIR)
		{
			checkToMap(&player);
		}

		player.die();
	}

	else
	{
		player.thinkTime--;

		game.drawScreen = FALSE;

		if (player.thinkTime <= 0)
		{
			player.action = &resetPlayer;
		}

		player.dirY = 0;
	}
}

static void resetPlayer()
{
	game.drawScreen = TRUE;

	player.draw = &drawLoopingAnimationToMap;

	getCheckpoint(&player.x, &player.y);

	if (game.canContinue == FALSE)
	{
		centerMapOnEntity(&player);

		cameraSnapToTargetEntity();
	}

	player.action = NULL;

	player.touch = &touch;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	if (game.infiniteEnergy == FALSE)
	{
		player.health -= (player.environment == SLIME ? 2 : 1);
	}

	if (player.health <= 0)
	{
		removeInventoryItemByObjectiveName("Amulet of Resurrection");

		player.health = player.maxHealth;

		setInfoBoxMessage(60, 255, 255, 255, _("Used Amulet of Resurrection"));
	}

	player.flags &= ~(HELPLESS|NO_DRAW|FLY);

	player.dirX = player.dirY = 0;

	player.y--;

	setCustomAction(&player, &invulnerable, 60, 0, 0);
}

void increasePlayerMaxHealth()
{
	player.maxHealth++;

	player.health = player.maxHealth;

	setInfoBoxMessage(60, 255, 255, 255, _("Maximum health has increased!"));

	if (player.health == 10)
	{
		addMedal("10_hp");
	}
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

	setEntityAnimation(&player, "DIE");
	setEntityAnimation(&playerShield, "DIE");
	setEntityAnimation(&playerWeapon, "DIE");

	player.frameSpeed = 1;

	player.health = 0;

	/*player.flags &= ~NO_DRAW;*/

	player.flags |= HELPLESS;

	playerWeapon.flags &= ~(ATTACKING|FLY);

	player.dirX = 0;

	player.thinkTime = 120;

	if (player.element == ICE || player.environment == LAVA || player.environment == SLIME)
	{
		player.flags |= NO_DRAW;
		playerShield.flags |= NO_DRAW;
		playerWeapon.flags |= NO_DRAW;

		if (player.environment != SLIME)
		{
			removeInventoryItemByObjectiveName("Amulet of Resurrection");
		}
	}

	if (getInventoryItemByObjectiveName("Amulet of Resurrection") == NULL)
	{
		player.action = &gameOverTimeOut;

		doGameOver();

		loadGameOverMusic();
	}

	else
	{
		player.action = &resurrectionTimeOut;
	}
}

static void resurrectionTimeOut()
{
	int i;
	Entity *e;

	player.thinkTime--;

	player.flags |= INVULNERABLE;

	if (player.thinkTime == 0)
	{
		for (i=0;i<6;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Resurrection particle");
			}

			loadProperties("boss/awesome_fireball_particle", e);

			setEntityAnimation(e, "STAND");

			e->head = &player;

			e->x = player.x + player.w / 2 - e->w / 2;
			e->y = player.y + player.h / 2 - e->h / 2;

			e->startX = e->x;
			e->startY = e->y;

			e->draw = &drawLoopingAnimationToMap;

			e->mental = 180;

			e->health = i * 60;

			e->action = &resurrectionParticleWait;
		}

		player.action = &resurrectionWait;

		player.mental = 6;

		player.health = player.maxHealth;
	}

	checkToMap(self);
}

static void resurrectionWait()
{
	if (player.mental == 0)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("Used Amulet of Resurrection"));

		removeInventoryItemByObjectiveName("Amulet of Resurrection");

		setEntityAnimation(&player, "STAND");
		setEntityAnimation(&playerShield, "STAND");
		setEntityAnimation(&playerWeapon, "STAND");

		player.action = NULL;

		setCustomAction(&player, &invulnerable, 180, 0, 0);
	}

	checkToMap(self);
}

static void gameOverTimeOut()
{
	player.thinkTime--;

	checkToMap(&player);
}

void freePlayer()
{
	memset(&player, 0, sizeof(Entity));
	memset(&playerWeapon, 0, sizeof(Entity));
	memset(&playerShield, 0, sizeof(Entity));
}

static void touch(Entity *other)
{

}

EntityList *playerGib()
{
	EntityList *list = NULL;

	/* Don't multigib */

	if (player.health > 0)
	{
		/* Don't allow resurrecting */

		removeInventoryItemByObjectiveName("Amulet of Resurrection");

		list = throwGibs("edgar/edgar_gibs", 6);

		player.inUse = TRUE;

		playerDie();

		player.flags |= NO_DRAW;
	}

	return list;
}

void facePlayer()
{
	self->face = player.x < self->x ? LEFT : RIGHT;
}

void setPlayerStunned(int thinkTime)
{
	if (player.health <= 0)
	{
		return;
	}

	/* Change back to Edgar */

	if (player.element == WATER)
	{
		becomeEdgar();
	}

	if (player.action != NULL)
	{
		player.action = NULL;

		if (player.target == NULL)
		{
			player.target = NULL;
		}
	}

	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	player.flags &= ~BLOCKING;

	setCustomAction(&player, &dizzy, thinkTime, 0, 0);

	setEntityAnimation(&player, "DIE");
	setEntityAnimation(&playerShield, "DIE");
	setEntityAnimation(&playerWeapon, "DIE");

	player.dirX = 0;
}

void doStunned()
{
	player.flags &= ~BLOCKING;

	setEntityAnimation(&player, "DIE");
	setEntityAnimation(&playerShield, "DIE");
	setEntityAnimation(&playerWeapon, "DIE");
}

void setPlayerSlimed(int thinkTime)
{
	Entity *e = NULL;

	if (player.health <= 0)
	{
		return;
	}

	e = getFreeEntity();

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

	setEntityAnimation(e, "STAND");

	e->thinkTime = thinkTime;

	player.dirX = 0;

	setCustomAction(&player, &helpless, thinkTime, 0, 0);

	player.flags &= ~BLOCKING;

	setEntityAnimation(&player, "STAND");
	setEntityAnimation(&playerWeapon, "STAND");
	setEntityAnimation(&playerShield, "STAND");

	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);
}

static void applySlime()
{
	if (player.action == NULL)
	{
		self->thinkTime--;
	}

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
	setEntityAnimation(self, "ATTACK_1");
	setEntityAnimation(&playerShield, "ATTACK_1");
	setEntityAnimation(&playerWeapon, "ATTACK_1");

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
		setEntityAnimation(&player, "ATTACK_2");
		setEntityAnimation(&playerShield, "ATTACK_1");
		setEntityAnimation(&playerWeapon, "ATTACK_1");

		playerWeapon.animationCallback = &fireArrow;
	}
}

static void fireArrow()
{
	Entity *arrow, *e;

	setEntityAnimation(&player, "ATTACK_2");
	setEntityAnimation(&playerShield, "ATTACK_1");
	setEntityAnimation(&playerWeapon, "ATTACK_2");

	arrow = getInventoryItemByObjectiveName(playerWeapon.requires);

	if (arrow != NULL)
	{
		e = addProjectile(arrow->name, &playerWeapon, playerWeapon.x + (player.face == RIGHT ? 0 : player.w), player.y + 15, player.face == RIGHT ? arrow->speed : -arrow->speed, 0);

		if (e->face == LEFT)
		{
			e->x -= e->w;
		}

		playSoundToMap("sound/edgar/arrow.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

		e->reactToBlock = &bounceOffShield;

		e->face = player.face;

		e->flags |= FLY|ATTACKING;

		if (game.infiniteArrows == FALSE)
		{
			arrow->health--;
		}

		if (arrow->health <= 0)
		{
			removeInventoryItemByObjectiveName(playerWeapon.requires);
		}

		game.arrowsFired++;

		if (game.arrowsFired == 250)
		{
			addMedal("arrow_250");
		}

		else if (game.arrowsFired == 500)
		{
			addMedal("arrow_500");
		}
	}

	else
	{
		setInfoBoxMessage(30, 255, 255, 255, _("Out of arrows!"));
	}

	playerWeapon.animationCallback = &attackFinish;
}

void setBowAmmo(int val)
{
	Entity *bow = getInventoryItemByObjectiveName("Bow");

	if (bow != NULL)
	{
		if (game.status == IN_INVENTORY)
		{
			setInventoryDialogMessage(_("Bow will now fire %s"), _(self->objectiveName));
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Bow will now fire %s"), _(self->objectiveName));
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
			setInfoBoxMessage(60, 255, 255, 255, _("A bow is required to use this item"));
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

	if (isValidOnMap(&player) == FALSE || isNearObstacle(&player) == TRUE)
	{
		loadProperties("edgar/edgar", &player);

		player.x = originalX;
		player.y = originalY;

		setInfoBoxMessage(60, 255, 255, 255, _("Cannot transmogrify here..."));

		player.type = PLAYER;
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

		setCustomAction(&player, &slimeTimeout, 60 * seconds, 0, 0);
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

	player.type = PLAYER;

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

void setPlayerLocked(int lock)
{
	if (lock == TRUE)
	{
		player.action = &playerWait;

		player.animationCallback = NULL;
		playerShield.animationCallback = NULL;
		playerWeapon.animationCallback = NULL;

		playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

		setEntityAnimation(&player, "STAND");
		setEntityAnimation(&playerShield, "STAND");
		setEntityAnimation(&playerWeapon, "STAND");

		player.dirX = 0;
	}

	else
	{
		if (player.health > 0)
		{
			player.action = NULL;
		}
	}
}

int isPlayerLocked()
{
	return player.action == NULL ? FALSE : TRUE;
}

static void playerWait()
{
	checkToMap(self);
}

Entity *removePlayerWeapon()
{
	Entity *e;

	if (playerWeapon.inUse == TRUE)
	{
		removeInventoryItemByObjectiveName(playerWeapon.objectiveName);

		e = addPermanentItem(playerWeapon.name, self->x, self->y);

		playerWeapon.inUse = FALSE;
	}

	else
	{
		e = NULL;
	}

	return e;
}

Entity *removePlayerShield()
{
	Entity *e;

	if (playerShield.inUse == TRUE)
	{
		removeInventoryItemByObjectiveName(playerShield.objectiveName);

		e = addPermanentItem(playerShield.name, self->x, self->y);

		playerShield.inUse = FALSE;
	}

	else
	{
		e = NULL;
	}

	return e;
}

void setPlayerFrozen(int thinkTime)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Iced Player");
	}

	/* Change back to Edgar */

	if (player.element == WATER)
	{
		becomeEdgar();
	}

	player.element = ICE;

	loadProperties("edgar/edgar_frozen", e);

	playSoundToMap("sound/common/freeze.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

	e->x = player.x + player.w / 2;
	e->y = player.y + player.h / 2;

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->type = ENEMY;

	e->face = player.face;

	e->action = &applyIce;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = thinkTime;

	player.dirX = 0;

	setCustomAction(&player, &helpless, thinkTime, 0, 0);

	player.flags &= ~BLOCKING;

	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	setEntityAnimation(&player, "STAND");
	setEntityAnimation(&playerShield, "STAND");
	setEntityAnimation(&playerWeapon, "STAND");
}

static void applyIce()
{
	int i;
	Entity *e;

	self->thinkTime--;

	self->face = player.face;

	if (self->thinkTime <= 0 || player.health <= 0 || (player.flags & TELEPORTING))
	{
		for (i=0;i<8;i++)
		{
			e = addTemporaryItem("common/ice_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}

		playSoundToMap("sound/common/shatter.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

		self->inUse = FALSE;

		player.element = NO_ELEMENT;
	}

	else
	{
		player.dirX = 0;

		self->x = player.x + player.w / 2 - self->w / 2;
		self->y = player.y + player.h / 2 - self->h / 2;
	}
}

void setPlayerWrapped(int thinkTime)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Wrapped Player");
	}

	/* Change back to Edgar */

	if (player.element == WATER)
	{
		becomeEdgar();
	}

	loadProperties("edgar/edgar_wrapped", e);

	e->x = player.x + player.w / 2;
	e->y = player.y + player.h / 2;

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->type = ENEMY;

	e->face = player.face;

	e->action = &applyWebbing;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = thinkTime;

	player.dirX = 0;

	setCustomAction(&player, &helpless, thinkTime, 0, 0);

	player.flags |= WRAPPED;

	player.flags &= ~BLOCKING;

	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	setEntityAnimation(&player, "STAND");
	setEntityAnimation(&playerShield, "STAND");
	setEntityAnimation(&playerWeapon, "STAND");
}

static void applyWebbing()
{
	int i;
	Entity *e;

	self->thinkTime--;

	self->face = player.face;

	player.dirX = 0;

	self->x = player.x + player.w / 2;
	self->y = player.y + player.h / 2;

	self->x -= self->w / 2;
	self->y -= self->h / 2;

	if (self->thinkTime <= 0 || player.health <= 0)
	{
		for (i=0;i<4;i++)
		{
			e = addTemporaryItem("common/web_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}

		self->inUse = FALSE;

		player.flags &= ~WRAPPED;
	}

	checkToMap(self);
}

int isAttacking()
{
	return (playerWeapon.flags & ATTACKING) ? TRUE : FALSE;
}

void playerStand()
{
	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	setEntityAnimation(&player, "STAND");
	setEntityAnimation(&playerShield, "STAND");
	setEntityAnimation(&playerWeapon, "STAND");
}

static void weaponTouch(Entity *other)
{

}

static void lightningSwordTouch(Entity *other)
{
	if (other->takeDamage != NULL && self->thinkTime <= 0)
	{
		self->mental--;

		self->head->mental = self->mental;

		if (self->mental == 10)
		{
			freeMessageQueue();

			setInfoBoxMessage(60, 255, 255, 255, _("10 charges remaining..."));
		}

		else if (self->mental <= 0)
		{
			freeMessageQueue();

			setInfoBoxMessage(120, 255, 255, 255, _("%s is out of power"), self->objectiveName);

			self->touch = NULL;

			self->mental = -1;
		}

		self->thinkTime = HIT_INVULNERABLE_TIME;
	}
}

static void lightningSwordChangeToBasic()
{
	loadProperties("weapon/lightning_sword_empty", &playerWeapon);

	playerWeapon.touch = &weaponTouch;

	alignAnimations(&playerWeapon);

	playerWeapon.action = &swingSword;

	playerWeapon.mental = -2;

	replaceInventoryItemWithName("weapon/lightning_sword", &playerWeapon);
}

void addChargesToWeapon()
{
	int mental = 0;
	Entity *e;

	if (strcmpignorecase(playerWeapon.name, "weapon/lightning_sword") == 0 ||
		strcmpignorecase(playerWeapon.name, "weapon/lightning_sword_empty") == 0)
	{
		mental = playerWeapon.mental;

		if (playerWeapon.mental < 0)
		{
			playerWeapon.mental = 0;
		}

		playerWeapon.mental += self->health;

		if (playerWeapon.mental > SWORD_MAX_CHARGE)
		{
			playerWeapon.mental = SWORD_MAX_CHARGE;
		}

		playerWeapon.head->mental = playerWeapon.mental;

		/* Transform back into Lightning Sword */

		if (mental == -2)
		{
			mental = playerWeapon.mental;

			loadProperties("weapon/lightning_sword", &playerWeapon);

			playerWeapon.mental = mental;

			playerWeapon.touch = &lightningSwordTouch;

			alignAnimations(&playerWeapon);

			playerWeapon.action = &swingSword;

			replaceInventoryItemWithName("weapon/lightning_sword_empty", &playerWeapon);

			setInfoBoxMessage(60, 255, 255, 255, _("%s has regained power"), _(playerWeapon.objectiveName));
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Picked up %s x %d"), _(self->objectiveName), self->health);
		}
	}

	else
	{
		e = getInventoryItemByObjectiveName(self->requires);

		if (e != NULL)
		{
			mental = e->mental;

			if (e->mental < 0)
			{
				e->mental = 0;
			}

			e->mental += self->health;

			if (e->mental > SWORD_MAX_CHARGE)
			{
				e->mental = SWORD_MAX_CHARGE;
			}

			/* Transform back into Lightning Sword */

			if (mental == -2)
			{
				mental = e->mental;

				loadProperties("weapon/lightning_sword", e);

				e->mental = mental;

				e->touch = &lightningSwordTouch;

				setInfoBoxMessage(60, 255, 255, 255, _("%s has regained power"), _(e->objectiveName));
			}

			else
			{
				setInfoBoxMessage(60, 255, 255, 255, _("Picked up %s x %d"), _(self->objectiveName), self->health);
			}
		}
	}
}

static void resurrectionParticleWait()
{
	float radians;

	self->mental -= 2;

	if (self->mental <= 0)
	{
		self->head->mental--;

		self->inUse = FALSE;
	}

	self->health += 8;

	radians = DEG_TO_RAD(self->health);

	self->x = (0 * cos(radians) - self->mental * sin(radians));
	self->y = (0 * sin(radians) + self->mental * cos(radians));

	self->x += self->startX;
	self->y += self->startY;
}

void setWeaponFromScript(char *name)
{
	self = getInventoryItemByName(name);

	if (self == NULL)
	{
		showErrorAndExit("Could not find inventory item %s\n", name);
	}

	setPlayerWeapon(1);
}

void setShieldFromScript(char *name)
{
	self = getInventoryItemByName(name);

	if (self == NULL)
	{
		showErrorAndExit("Could not find inventory item %s\n", name);
	}

	setPlayerShield(1);
}

void scriptAttack()
{
	playerWeapon.flags |= ATTACKING;

	playerWeapon.action();
}
