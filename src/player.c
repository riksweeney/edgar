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

extern Entity player, playerShield, playerWeapon;
extern Entity *self;
extern Input input;
extern Game game;

static void takeDamage(Entity *, int);
static void attackFinish(void);
static void resetPlayer(void);
static void fallout(void);
static void falloutPause(void);
static void resetPause(void);
static void resetPlayer(void);
static void dialogWait(void);
static void alignAnimations(Entity *);

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

		player.fallout = &fallout;
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

	/* Gravity always pulls the player down */

	if (!(self->flags & FLY))
	{
		switch (self->environment)
		{
			case WATER:
				self->dirY += (self->flags & FLOATS) ? -GRAVITY_SPEED * 0.6 : GRAVITY_SPEED * 0.25;

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
				self->dirY += GRAVITY_SPEED;

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

	playerWeapon.frameSpeed = player.frameSpeed;
	playerShield.frameSpeed = player.frameSpeed;

	if (self->action == NULL)
	{
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

			if (self->standingOn != NULL)
			{
				self->dirX += self->standingOn->dirX;

				if (self->standingOn->dirY > 0)
				{
					self->dirY = self->standingOn->dirY + 1;
				}
			}

			if (!(playerWeapon.flags & ATTACKING))
			{
				if (input.left == 1)
				{
					if (player.flags & BLOCKING)
					{
						playerWeapon.face = playerShield.face = self->face = LEFT;
					}

					else
					{
						self->dirX -= self->speed;

						/* Only pull the target */

						if ((self->flags & GRABBING) && self->target != NULL)
						{
							if (self->target->x > self->x)
							{
								self->target->dirX = -self->speed;

								self->target->frameSpeed = -1;
							}
						}

						else
						{
							playerWeapon.face = playerShield.face = self->face = LEFT;
						}

						setEntityAnimation(&player, WALK);
						setEntityAnimation(&playerShield, WALK);
						setEntityAnimation(&playerWeapon, WALK);
					}
				}

				else if (input.right == 1)
				{
					if (player.flags & BLOCKING)
					{
						playerWeapon.face = playerShield.face = self->face = RIGHT;
					}

					else
					{
						self->dirX += self->speed;

						/* Only pull the target */

						if ((self->flags & GRABBING) && self->target != NULL)
						{
							if (self->target->x < self->x)
							{
								self->target->dirX = self->speed;

								self->target->frameSpeed = 1;
							}
						}

						else
						{
							playerWeapon.face = playerShield.face = self->face = RIGHT;
						}

						setEntityAnimation(&player, WALK);
						setEntityAnimation(&playerShield, WALK);
						setEntityAnimation(&playerWeapon, WALK);
					}
				}

				else if (input.left == 0 && input.right == 0 && !(player.flags & BLOCKING))
				{
					setEntityAnimation(&player, STAND);
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
					if (self->flags & FLY)
					{
						self->dirY = -1;
					}

					else
					{
						if (self->standingOn != NULL)
						{
							if (self->standingOn->activate != NULL)
							{
								printf("Trying to activate %s\n", self->standingOn->objectiveName);

								self = self->standingOn;

								self->activate(1);

								self = &player;
							}

							input.up = 0;
						}
					}
				}

				if (input.down == 1)
				{
					if (self->flags & FLY)
					{
						self->dirY = 1;
					}

					else
					{
						if (self->standingOn != NULL)
						{
							if (self->standingOn->activate != NULL)
							{
								printf("Trying to activate %s\n", self->standingOn->objectiveName);

								self = self->standingOn;

								self->activate(-1);

								self = &player;

								self->dirY = self->standingOn->speed;
							}

							input.down = 0;
						}
					}
				}

				if (input.attack == 1 && !(player.flags & BLOCKING) && !(player.flags & GRABBED))
				{
					if (playerWeapon.inUse == TRUE)
					{
						playerWeapon.flags |= ATTACKING;

						setEntityAnimation(&player, ATTACK_1);
						setEntityAnimation(&playerShield, ATTACK_1);
						setEntityAnimation(&playerWeapon, ATTACK_1);

						playerWeapon.animationCallback = &attackFinish;
					}

					input.attack = 0;
				}

				if (input.interact == 1)
				{
					interactWithEntity(self->x, self->y, self->w, self->h);

					input.interact = 0;
				}

				if (input.grabbing == 1 && !(player.flags & BLOCKING))
				{
					self->flags |= GRABBING;
				}

				else
				{
					self->flags &= ~GRABBING;

					if (self->target != NULL)
					{
						self->target->flags &= ~HELPLESS;

						self->target->frameSpeed = 0;

						self->target = NULL;
					}
				}

				if (input.block == 1 && playerShield.inUse == TRUE && !(playerWeapon.flags & ATTACKING) && !(player.flags & GRABBED))
				{
					player.flags |= BLOCKING;

					setEntityAnimation(&player, BLOCK);
					setEntityAnimation(&playerShield, BLOCK);

					playerShield.thinkTime++;
				}

				else if ((input.block == 0 && (player.flags & BLOCKING)))
				{
					player.flags &= ~BLOCKING;

					setEntityAnimation(&player, STAND);
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
					selectNextInventoryItem(input.next == 1 ? 1 : -1);

					input.next = input.previous = 0;
				}

				if (input.jump == 1 && !(player.flags & BLOCKING))
				{
					if (self->flags & ON_GROUND)
					{
						self->dirY = -JUMP_HEIGHT;
					}

					input.jump = 0;
				}
				#if DEV == 1
					if (input.fly == 1)
					{
						self->flags ^= FLY;

						input.fly = 0;
					}
				#endif
			}
		}

		else
		{
			self->flags &= ~GRABBING;

			if (self->target != NULL)
			{
				self->target->flags &= ~HELPLESS;

				self->target = NULL;
			}
		}

		i = player.environment;

		checkToMap(self);

		self->standingOn = NULL;

		i = mapTileAt(self->x / TILE_SIZE, (self->y + self->h + 5) / TILE_SIZE);
		j = mapTileAt((self->x + self->w - 1) / TILE_SIZE, (self->y + self->h + 5) / TILE_SIZE);

		if ((self->flags & ON_GROUND) && i > BLANK_TILE && i < BACKGROUND_TILE_START && j > BLANK_TILE && j < BACKGROUND_TILE_START)
		{
			setCheckpoint(self->x, self->y);
		}
	}

	else
	{
		self->action();
	}

	addToGrid(&player);

	if (playerWeapon.flags & ATTACKING)
	{
		addToGrid(&playerWeapon);
	}
}

void playerWaitForDialog()
{
	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerShield, STAND);
	setEntityAnimation(&playerWeapon, STAND);

	player.dirX = 0;

	player.action = &dialogWait;
}

void playerResumeNormal()
{
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
}

void drawPlayer()
{
	self = &player;

	if (self->inUse == TRUE && (self->flags & NO_DRAW) == 0)
	{
		if (!(self->flags & BLOCKING))
		{
			/* Draw the weapon */

			self = &playerWeapon;

			if (self->inUse == TRUE)
			{
				self->x = player.x;
				self->y = player.y;

				self->draw();
			}
		}

		/* Draw the player */

		self = &player;

		self->draw();

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

void setPlayerShield(int val)
{
	playerShield = *self;

	alignAnimations(&playerShield);
}

void setPlayerWeapon(int val)
{
	playerWeapon = *self;

	alignAnimations(&playerWeapon);
}

void autoSetPlayerWeapon(Entity *newWeapon)
{
	if (playerWeapon.inUse == FALSE)
	{
		playerWeapon = *newWeapon;

		alignAnimations(&playerWeapon);
	}
}

void autoSetPlayerShield(Entity *newWeapon)
{
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

	if (other->dirX != 0 && (player.flags & BLOCKING) && !(other->flags & UNBLOCKABLE))
	{
		if (other->type == PROJECTILE)
		{
			if ((other->dirX > 0 && player.face == LEFT) || (other->dirX < 0 && player.face == RIGHT))
			{
				if (other->element == NO_ELEMENT || (playerShield.element == other->element))
				{
					player.dirX = other->dirX < 0 ? -2 : 2;

					checkToMap(&player);

					if (other->reactToBlock != NULL)
					{
						temp = self;

						self = other;

						self->reactToBlock();

						self = temp;

						return;
					}
					
					other->inUse = FALSE;
		
					return;
				}
			}
		}

		else if ((other->dirX > 0 && player.face == LEFT) || (other->dirX < 0 && player.face == RIGHT))
		{
			player.dirX = other->dirX < 0 ? -2 : 2;

			checkToMap(&player);

			if (other->reactToBlock == NULL)
			{
				other->x = other->x < player.x ? player.x - other->w : player.x + player.w;

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

		setEntityAnimation(&player, STAND);
		setEntityAnimation(&playerShield, STAND);
		setEntityAnimation(&playerWeapon, STAND);

		if (other->type == PROJECTILE)
		{
			other->inUse = FALSE;
		}

		if (player.health > 0)
		{
			setCustomAction(&player, &helpless, 10, 0);

			setCustomAction(&player, &invulnerable, 60, 0);

			if (player.dirX == 0)
			{
				player.dirX = other->dirX < 0 ? -6 : 6;
			}

			else
			{
				player.dirX = player.dirX < 0 ? 6 : -6;
			}
		}

		else
		{
			printf("Game Over\n");

			exit(0);
		}
	}
}

int getDistanceFromPlayer(Entity *e)
{
	return getDistance(player.x, player.y, e->x + e->w, e->y);
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
	fprintf(fp, "SPEED %0.1f\n", self->speed);
	fprintf(fp, "WEIGHT %0.2f\n", self->weight);
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
		fprintf(fp, "SPEED %0.1f\n", self->speed);
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
		fprintf(fp, "SPEED %0.1f\n", self->speed);
		fprintf(fp, "WEIGHT %0.2f\n", self->weight);
		fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
		fprintf(fp, "REQUIRES %s\n", self->requires);
		fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
		fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
		fprintf(fp, "}\n\n");
	}
}

static void fallout()
{
	if (!(player.flags & HELPLESS))
	{
		centerMapOnEntity(NULL);

		player.thinkTime = 120;

		player.dirX = 0;

		player.flags |= HELPLESS;

		player.action = &falloutPause;

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

static void resetPlayer()
{
	game.drawScreen = TRUE;

	player.draw = &drawLoopingAnimationToMap;

	getCheckpoint(&player.x, &player.y);
	
	centerMapOnEntity(&player);

	cameraSnapToTargetEntity();

	printf("Respawned at %f %f\n", player.x, player.y);

	player.action = NULL;

	playerWeapon.flags &= ~(ATTACKING|ATTACK_SUCCESS);

	player.health--;

	if (player.health <= 0)
	{
		printf("Game over\n");

		exit(0);
	}

	player.flags &= ~HELPLESS;

	player.dirX = player.dirY = 0;

	player.y--;

	setCustomAction(&player, &invulnerable, 60, 0);
}

void increasePlayerMaxHealth()
{
	player.maxHealth++;

	player.health = player.maxHealth;

	setInfoBoxMessage(120,  _("Maximum health has increased!"));
}

void syncWeaponShieldToPlayer()
{
	playerWeapon.face = playerShield.face = player.face;
	playerWeapon.face = playerShield.face = player.face;

	setEntityAnimation(&playerWeapon, getAnimationTypeAtIndex(&player));
	setEntityAnimation(&playerShield, getAnimationTypeAtIndex(&player));
}
