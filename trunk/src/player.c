#include "player.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void checkToMap(Entity *);
extern void loadProperties(char *, Entity *);
extern void centerMapOnEntity(Entity *);
extern void dropInventoryItem(void);
extern void selectNextInventoryItem(int);
extern void helpless(int *);
extern void invulnerable(int *);
extern void setCustomAction(Entity *, void (*)(int *), int);
extern void useInventoryItem(void);
extern int getDistance(int, int, int, int);

void setPlayerLocation(int, int);

static void takeDamage(Entity *, int);
static void attackFinish(void);

void loadPlayer(int x, int y)
{
	loadProperties("edgar/edgar", &player);

	if (player.active != ACTIVE)
	{
		player.active = ACTIVE;
		player.x = x;
		player.y = y;
		player.dirX = player.dirY = 0;
		player.face = LEFT;
		player.flags = 0;
		player.type = PLAYER;

		player.thinkTime = 0;

		player.maxHealth = player.health = 5;

		setEntityAnimation(&player, STAND);

		player.draw = &drawLoopingAnimationToMap;

		player.takeDamage = &takeDamage;

		playerShield.parent = &player;
		playerWeapon.parent = &player;

		playerWeapon.face = playerShield.face = LEFT;

		centerMapOnEntity(&player);
	}

	else
	{
		setPlayerLocation(x, y);
	}
}

void setPlayerLocation(int x, int y)
{
	player.x = x;
	player.y = y;

	player.draw = &drawLoopingAnimationToMap;

	player.active = ACTIVE;
}

void doPlayer()
{
	int i;

	self = &player;

	if (self->action == NULL)
	{
		for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
		{
			if (self->customThinkTime[i] > 0)
			{
				self->custom[i](&self->customThinkTime[i]);
			}
		}

		/* Gravity always pulls the player down */

		if (!(self->flags & FLY))
		{
			self->dirY += GRAVITY_SPEED;

			if (self->dirY >= MAX_FALL_SPEED)
			{
				self->dirY = MAX_FALL_SPEED;
			}
		}

		else
		{
			self->dirY = 0;
		}

		if (!(self->flags & HELPLESS))
		{
			self->dirX = 0;

			if (self->standingOn != NULL)
			{
				self->dirX += self->standingOn->dirX;
			}

			if (self->animationCallback == NULL)
			{
				if (input.left == 1)
				{
					self->dirX -= PLAYER_SPEED;
					playerWeapon.face = playerShield.face = self->face = LEFT;

					setEntityAnimation(&player, WALK);
					setEntityAnimation(&playerShield, WALK);
					setEntityAnimation(&playerWeapon, WALK);
				}

				else if (input.right == 1)
				{
					self->dirX += PLAYER_SPEED;
					playerWeapon.face = playerShield.face = self->face = RIGHT;

					setEntityAnimation(&player, WALK);
					setEntityAnimation(&playerShield, WALK);
					setEntityAnimation(&playerWeapon, WALK);
				}

				else if (input.left == 0 && input.right == 0)
				{
					setEntityAnimation(&player, STAND);
					setEntityAnimation(&playerShield, STAND);
					setEntityAnimation(&playerWeapon, STAND);
				}

				if (input.up == 1 && self->flags & FLY)
				{
					self->dirY = -1;
				}

				if (input.down == 1 && self->flags & FLY)
				{
					self->dirY = 1;
				}

				if (input.attack == 1)
				{
					if (playerWeapon.active == ACTIVE)
					{
						player.animationCallback = &attackFinish;
						playerShield.animationCallback = &attackFinish;
						playerWeapon.animationCallback = &attackFinish;

						player.flags |= ATTACKING;
						playerShield.flags |= ATTACKING;
						playerWeapon.flags |= ATTACKING;

						setEntityAnimation(&player, ATTACK_1);
						setEntityAnimation(&playerShield, ATTACK_1);
						setEntityAnimation(&playerWeapon, ATTACK_1);
					}

					input.attack = 0;
				}

				if (input.drop == 1)
				{
					dropInventoryItem();

					input.drop = 0;
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

				if (input.fly == 1)
				{
					self->flags ^= FLY;

					input.fly = 0;
				}
			}
		}

		checkToMap(self);

		self->standingOn = NULL;
	}

	else
	{
		self->action();
	}
}

static void attackFinish()
{
	player.animationCallback = NULL;
	playerShield.animationCallback = NULL;
	playerWeapon.animationCallback = NULL;

	player.flags &= ~ATTACKING;
	playerShield.flags &= ~ATTACKING;
	playerWeapon.flags &= ~ATTACKING;

	setEntityAnimation(&player, STAND);
	setEntityAnimation(&playerShield, STAND);
	setEntityAnimation(&playerWeapon, STAND);
}

void drawPlayer()
{
	self = &player;

	if (self->active == ACTIVE && (self->flags & NO_DRAW) == 0)
	{
		/* Draw the weapon */

		self = &playerWeapon;

		if (self->active == ACTIVE)
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

		if (self->active == ACTIVE)
		{
			self->x = player.x;
			self->y = player.y;

			self->draw();
		}
	}
}

void setPlayerShield()
{
	playerShield = *self;

	playerShield.parent = &player;

	playerShield.face = player.face;
}

void setPlayerWeapon()
{
	playerWeapon = *self;

	playerWeapon.parent = &player;

	playerWeapon.face = player.face;
}

void autoSetPlayerWeapon(Entity *newWeapon)
{
	if (playerWeapon.active == INACTIVE)
	{
		playerWeapon = *newWeapon;

		playerWeapon.parent = &player;

		playerWeapon.face = player.face;
	}
}

void autoSetPlayerShield(Entity *newWeapon)
{
	if (playerShield.active == INACTIVE)
	{
		playerShield = *newWeapon;

		playerShield.parent = &player;

		playerShield.face = player.face;
	}
}

void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		player.animationCallback = NULL;
		playerShield.animationCallback = NULL;
		playerWeapon.animationCallback = NULL;

		player.flags &= ~ATTACKING|BLOCKING;
		playerShield.flags &= ~ATTACKING|BLOCKING;
		playerWeapon.flags &= ~ATTACKING|BLOCKING;

		setEntityAnimation(self, WALK);

		if (self->health > 0)
		{
			setCustomAction(self, &helpless, 4);

			setCustomAction(self, &invulnerable, 60);

			if (self->dirX == 0)
			{
				self->dirX = other->dirX < 0 ? -30 : 30;
			}

			else
			{
				self->dirX = self->dirX < 0 ? 30 : -30;
			}
		}
	}
}

int getDistanceFromPlayer(Entity *e)
{
	if (player.x < e->x)
	{
		return -getDistance(player.x + player.w, player.y, e->x, e->y);
	}
	
	return getDistance(player.x, player.y, e->x + e->w, e->y);
}
