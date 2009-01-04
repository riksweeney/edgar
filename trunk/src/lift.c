#include "lift.h"

extern Entity *getFreeEntity(void);
extern void drawLoopingAnimationToMap(void);
extern void setEntityAnimation(Entity *, int);
extern int collision(int, int, int, int, int, int, int, int);
extern void loadProperties(char *, Entity *);

static void touch(Entity *);
static void move(void);

void addLift(int startX, int startY, int endX, int endY, int pauseTime)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Lift!\n");
		
		return;
	}
	
	loadProperties("lift", e);
	
	e->type = LIFT;
	e->action = &move;
	e->touch = &touch;
	e->draw = &drawLoopingAnimationToMap;
	
	e->thinkTime = pauseTime;
	e->health = pauseTime;
	
	setEntityAnimation(e, STAND);
	
	e->startX = startX * TILE_SIZE;
	e->startY = startY * TILE_SIZE;
	
	e->endX = endX * TILE_SIZE;
	e->endY = endY * TILE_SIZE;
	
	e->x = e->startX;
	e->y = e->startY;
	
	e->targetX = e->endX;
	e->targetY = e->endY;
}

static void touch(Entity *other)
{
	if (other->flags & PUSHABLE)
	{
		/* Now test the vertical movement */
		
		if (other->dirY > 0)
		{
			/* Trying to move down */
			
			if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
			{
				/* Place the player as close to the solid tile as possible */
				
				other->y = self->y;
				other->y -= other->h;
				
				other->standingOn = self;
				other->flags |= ON_GROUND;
				other->dirY = 0;
			}
		}
	}
}

static void move()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}
	
	else
	{
		self->dirX = self->dirY = self->thinkTime = 0;
		
		if (abs(self->x - self->targetX) > 2)
		{
			self->dirX += (self->x < self->targetX ? 2 : -2);
		}
		
		else
		{
			self->x = self->targetX;
		}
		
		if (abs(self->y - self->targetY) > 2)
		{
			self->dirY += (self->y < self->targetY ? 2 : -2);
		}
		
		else
		{
			self->y = self->targetY;
		}
		
		if (self->x == self->targetX && self->y == self->targetY)
		{
			self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
			self->targetY = (self->targetY == self->endY ? self->startY : self->endY);
			
			self->thinkTime = self->health;
		}
		
		self->x += self->dirX;
		self->y += self->dirY;
	}
}
