#include "player.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingEntityAnimation(void);
extern void checkToMap(Entity *);
extern void loadProperties(char *, Entity *);
extern void centerMapOnEntity(Entity *);

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
		
		player.draw = &drawLoopingEntityAnimation;
		
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
	
	player.draw = &drawLoopingEntityAnimation;
	
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
