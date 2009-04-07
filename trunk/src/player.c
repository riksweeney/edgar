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

		printf("Setting player start to %d %d\n", x, y);
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

	centerMapOnEntity(&player);
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

	if (self->action == NULL)
	{
		for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
		{
			if (self->customThinkTime[i] > 0)
			{
				self->custom[i](&self->customThinkTime[i]);
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

				else if (input.right == 1)
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

				else if (input.left == 0 && input.right == 0)
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

				if (input.attack == 1)
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

				if (input.grabbing == 1)
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

				if (input.jump == 1)
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

		if (player.environment == WATER && i == AIR)
		{
			player.action = &fallout;
		}

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
		/* Draw the weapon */

		self = &playerWeapon;

		if (self->inUse == TRUE)
		{
			self->x = player.x;
			self->y = player.y;

			self->draw();
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

	playerShield.parent = &player;

	playerShield.face = player.face;

	setEntityAnimation(&playerShield, player.currentAnim);

	playerShield.currentFrame = player.currentFrame;

	playerShield.frameTimer = player.frameTimer;
}

void setPlayerWeapon(int val)
{
	playerWeapon = *self;

	playerWeapon.parent = &player;

	playerWeapon.face = player.face;

	setEntityAnimation(&playerWeapon, player.currentAnim);

	playerWeapon.currentFrame = player.currentFrame;

	playerWeapon.frameTimer = player.frameTimer;
}

void autoSetPlayerWeapon(Entity *newWeapon)
{
	if (playerWeapon.inUse == FALSE)
	{
		playerWeapon = *newWeapon;

		playerWeapon.parent = &player;

		playerWeapon.face = player.face;

		playerWeapon.inUse = TRUE;

		setEntityAnimation(&playerWeapon, player.currentAnim);

		playerWeapon.currentFrame = player.currentFrame;

		playerWeapon.frameTimer = player.frameTimer;
	}
}

void autoSetPlayerShield(Entity *newWeapon)
{
	if (playerShield.inUse == FALSE)
	{
		playerShield = *newWeapon;

		playerShield.parent = &player;

		playerShield.face = player.face;

		playerShield.inUse = TRUE;

		setEntityAnimation(&playerShield, player.currentAnim);

		playerShield.currentFrame = player.currentFrame;

		playerShield.frameTimer = player.frameTimer;
	}
}

static void takeDamage(Entity *other, int damage)
{
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

		if (player.health > 0)
		{
			setCustomAction(&player, &helpless, 10);

			setCustomAction(&player, &invulnerable, 60);

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
	centerMapOnEntity(NULL);

	player.thinkTime = 120;

	player.dirX = 0;

	player.flags |= HELPLESS;

	player.action = &falloutPause;

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerShield, STAND);
	setEntityAnimation(&playerWeapon, STAND);

	checkToMap(&player);
}

static void falloutPause()
{
	player.thinkTime--;

	if (player.thinkTime <= 0)
	{
		player.thinkTime = 60;

		player.action = &resetPause;
	}

	checkToMap(&player);
}

static void resetPause()
{
	player.thinkTime--;

	game.drawScreen = FALSE;

	if (player.thinkTime <= 0)
	{
		player.action = &resetPlayer;
	}

	checkToMap(&player);
}

static void resetPlayer()
{
	centerMapOnEntity(&player);

	game.drawScreen = TRUE;

	player.draw = &drawLoopingAnimationToMap;

	getCheckpoint(&player.x, &player.y);

	printf("Respawned at %f %f\n", player.x, player.y);

	player.action = NULL;

	player.health--;

	player.flags &= ~HELPLESS;

	player.dirX = player.dirY = 0;

	player.y--;

	setCustomAction(&player, &invulnerable, 60);
}
