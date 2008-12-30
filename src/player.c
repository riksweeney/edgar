#include "player.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingEntityAnimation(void);
extern void checkToMap(Entity *);
extern void centerEntityOnMap(Entity *);
extern void loadProperties(char *, Entity *);

void loadPlayer()
{
	loadProperties("edgar", &player);
}

void initPlayer()
{
	player.x = 8;
	player.y = 0;
	player.dirX = player.dirY = 0;
	player.face = RIGHT;
	player.flags = 0;
	player.type = PLAYER;

	player.thinkTime = 0;
	
	setEntityAnimation(&player, STAND_RIGHT);
	
	player.draw = &drawLoopingEntityAnimation;
	
	playerShield.parent = &player;
	playerWeapon.parent = &player;
	
	playerWeapon.face = playerShield.face = RIGHT;
}

void setPlayerLocation(int x, int y)
{
	player.x = x;
	player.y = y;
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
		
		if (!(self->flags & HELPLESS))
		{
			self->dirX = 0;
			
			if (self->standingOn != NULL)
			{
				self->dirX += self->standingOn->dirX;
			}
		
			if (self->dirY >= MAX_FALL_SPEED)
			{
				self->dirY = MAX_FALL_SPEED;
			}
		
			if (input.left == 1)
			{
				self->dirX -= PLAYER_SPEED;
				playerWeapon.face = playerShield.face = self->face = LEFT;
				
				setEntityAnimation(&player, WALK_LEFT);
				setEntityAnimation(&playerShield, WALK_LEFT);
				setEntityAnimation(&playerWeapon, WALK_LEFT);
			}
		
			else if (input.right == 1)
			{
				self->dirX += PLAYER_SPEED;
				playerWeapon.face = playerShield.face = self->face = RIGHT;
				
				setEntityAnimation(&player, WALK_RIGHT);
				setEntityAnimation(&playerShield, WALK_RIGHT);
				setEntityAnimation(&playerWeapon, WALK_RIGHT);
			}
		
			else if (input.left == 0 && input.right == 0)
			{
				if (self->face == RIGHT)
				{
					setEntityAnimation(&player, STAND_RIGHT);
					setEntityAnimation(&playerShield, STAND_RIGHT);
					setEntityAnimation(&playerWeapon, STAND_RIGHT);
				}
		
				else if (self->face == LEFT)
				{
					setEntityAnimation(&player, STAND_LEFT);
					setEntityAnimation(&playerShield, STAND_LEFT);
					setEntityAnimation(&playerWeapon, STAND_LEFT);
				}
			}
		
			if (input.jump == 1)
			{
				if (self->flags & ON_GROUND)
				{
					/*
					if (self->face == RIGHT && self->state != STAND_RIGHT)
					{
						self->state = STAND_RIGHT;
			
						setEntityAnimation("PLAYER_STAND_RIGHT", &player);
					}
			
					else if (self->face == LEFT && self->state != STAND_LEFT)
					{
						self->state = STAND_LEFT;
			
						setEntityAnimation("PLAYER_STAND_LEFT", &player);
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
		
		checkToMap(&player);
		
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
	
	centerEntityOnMap(self);
	
	if ((self->flags & NO_DRAW) == 0)
	{
		/* Draw the weapon */
		
		self = &playerWeapon;
		
		self->x = player.x;
		self->y = player.y;
		
		self->draw();
		
		/* Draw the player */
		
		self = &player;
		
		self->draw();
		
		/* Draw the shield */
		
		self = &playerShield;
		
		self->x = player.x;
		self->y = player.y;
		
		self->draw();
	}
}
