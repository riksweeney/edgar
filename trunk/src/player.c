#include "player.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void checkToMap(Entity *);
extern void loadProperties(char *, Entity *);
extern void centerMapOnEntity(Entity *);
extern void dropInventoryItem(void);
extern void selectNextInventoryItem(int);

void setPlayerLocation(int, int);

void loadPlayer(int x, int y)
{
	loadProperties("edgar", &player);
	
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
		
		setEntityAnimation(&player, STAND);
		
		player.draw = &drawLoopingAnimationToMap;
		
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
		
		self->dirY += GRAVITY_SPEED;
		
		if (self->dirY >= MAX_FALL_SPEED)
		{
			self->dirY = MAX_FALL_SPEED;
		}
		
		if (!(self->flags & HELPLESS))
		{
			self->dirX = 0;
			
			if (self->standingOn != NULL)
			{
				self->dirX += self->standingOn->dirX;
			}
			
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
			
			if (input.drop == 1)
			{
				dropInventoryItem();
				
				input.drop = 0;
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
					/*
					if (self->face == RIGHT && self->state != STAND)
					{
						self->state = STAND;
			
						setEntityAnimation("PLAYER_STAND", &player);
					}
			
					else if (self->face == LEFT && self->state != STAND)
					{
						self->state = STAND;
			
						setEntityAnimation("PLAYER_STAND", &player);
					}
					*/
					self->dirY = -JUMP_HEIGHT;
				}
		
				input.jump = 0;
			}
			/*
			if (input.attack == 1)
			{
				if (!(self->flags & ATTACKING))
				{
					setEntityAnimation(strcat(getGameSword, "SWING_1"), playerSword);
					
					playerSword.draw = drawEntityNoLoopAnimation;
					playerSword.animCallback = 
				}
			}
			*/
		}
		
		checkToMap(self);
		
		self->standingOn = NULL;
	}
	
	else
	{
		self->action();
	}
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

Entity *getPlayer()
{
	return &player;
}

Entity *getPlayerShield()
{
	return &playerShield;
}

Entity *getPlayerWeapon()
{
	return &playerWeapon;
}

void setPlayerShield(Entity *shield, int onlyIfEmpty)
{
	if (onlyIfEmpty == 1)
	{
		if (playerShield.active == INACTIVE)
		{
			playerShield = *shield;
			
			playerShield.parent = &player;
			
			playerShield.face = player.face;
		}
	}
	
	else if (onlyIfEmpty == 0)
	{
		playerShield = *shield;
	}
}

void setPlayerWeapon(Entity *weapon, int onlyIfEmpty)
{
	if (onlyIfEmpty == 1)
	{
		if (playerWeapon.active == INACTIVE)
		{
			playerWeapon = *weapon;
			
			playerWeapon.parent = &player;
			
			playerWeapon.face = player.face;
		}
	}
	
	else if (onlyIfEmpty == 0)
	{
		playerWeapon = *weapon;
	}
}
