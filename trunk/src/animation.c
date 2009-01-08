#include "animation.h"

extern SDL_Surface *loadImage(char *);
extern void drawImage(SDL_Surface *, int, int);
extern void drawFlippedImage(SDL_Surface *, int, int);
extern SDL_Surface *getSpriteImage(int);
extern int mapStartX(void);
extern int mapStartY(void);

void loadAnimationData(char *filename)
{
	char frameName[255];
	int id, i;
	FILE *fp;
	
	id = -1;

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Failed to open animation file: %s\n", filename);

		exit(1);
	}
	
	printf("Loading animation data from %s\n", filename);

	while (!feof(fp))
	{		
		i = fscanf(fp, "%s", frameName);
		
		if (i == -1 || frameName[0] == '#' || frameName[0] == '\n')
		{
			continue;
		}
		
		if (strcmpignorecase(frameName, "INDEX") == 0)
		{
			if (id != -1)
			{
				if (animation[id].frameCount == 0)
				{
					printf("Animation %d was created with 0 frames\n", id);
					
					exit(1);
				}
			}
			
			fscanf(fp, "%d", &id);
		}
		
		else if (strcmpignorecase(frameName, "FRAMES") == 0)
		{
			if (animation[id].frameID != NULL)
			{
				printf("Attempting to overwrite existing animation in slot %d\n", id);
				
				exit(1);
			}
			
			fscanf(fp, "%d", &animation[id].frameCount);
	
			/* Allocate space for the frame timer */
	
			animation[id].frameTimer = (int *)malloc(animation[id].frameCount * sizeof(int));
	
			if (animation[id].frameTimer == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);
	
				exit(1);
			}
			
			/* Allocate space for the frame timer */
	
			animation[id].frameID = (int *)malloc(animation[id].frameCount * sizeof(int));
	
			if (animation[id].frameID == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);
	
				exit(1);
			}
			
			/* Allocate space for the offsets */
	
			animation[id].offsetX = (int *)malloc(animation[id].frameCount * sizeof(int));
	
			if (animation[id].offsetX == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);
	
				exit(1);
			}
			
			animation[id].offsetY = (int *)malloc(animation[id].frameCount * sizeof(int));
	
			if (animation[id].offsetY == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);
	
				exit(1);
			}
	
			/* Now load up each frame */
	
			for (i=0;i<animation[id].frameCount;i++)
			{
				fscanf(fp, "%d %d %d %d", &animation[id].frameID[i], &animation[id].frameTimer[i], &animation[id].offsetX[i], &animation[id].offsetY[i]);
			}
		}
	}
	
	if (animation[id].frameCount == 0)
	{
		printf("Animation %d was created with 0 frames\n", id);
		
		exit(1);
	}
}

void freeAnimation(Animation *anim)
{
	if (anim->frameTimer != NULL)
	{
		free(anim->frameTimer);
	}
	
	if (anim->frameID != NULL)
	{
		free(anim->frameID);
	}
	
	if (anim->offsetX != NULL)
	{
		free(anim->offsetX);
	}
	
	if (anim->offsetY != NULL)
	{
		free(anim->offsetY);
	}
}

void freeAnimations()
{
	int i;

	for (i=0;i<MAX_ANIMATIONS;i++)
	{
		freeAnimation(&animation[i]);
	}
}

void drawLoopingAnimation(Entity *e, int x, int y, int w, int h, int center)
{
	SDL_Surface *image;
	
	e->frameTimer--;

	if (e->frameTimer <= 0)
	{
		e->currentFrame++;

		if (e->currentFrame >= animation[e->currentAnim].frameCount)
		{
			e->currentFrame = 0;
		}

		e->frameTimer = animation[e->currentAnim].frameTimer[e->currentFrame];
		
		image = getSpriteImage(animation[e->currentAnim].frameID[e->currentFrame]);
		
		if (image == NULL)
		{
			printf("Image index %d is NULL!\n", animation[e->currentAnim].frameID[e->currentFrame]);
		}

		e->w = image->w;
		e->h = image->h;
	}
	
	else
	{
		image = getSpriteImage(animation[e->currentAnim].frameID[e->currentFrame]);
	}
	
	if (center == 1)
	{
		drawImage(image, x + (w - image->w) / 2, y + (h - image->h) / 2);
	}
	
	else
	{
		drawImage(image, x, y);
	}
}

void drawLoopingAnimationToMap()
{
	int x, y;
	SDL_Surface *image;
	
	self->frameTimer--;

	if (self->frameTimer <= 0)
	{
		self->currentFrame++;

		if (self->currentFrame >= animation[self->currentAnim].frameCount)
		{
			self->currentFrame = 0;
			
			if (self->animationCallback != NULL)
			{
				self->animationCallback();
				
				self->animationCallback = NULL;
			}
		}

		self->frameTimer = animation[self->currentAnim].frameTimer[self->currentFrame];
		
		image = getSpriteImage(animation[self->currentAnim].frameID[self->currentFrame]);
		
		if (image == NULL)
		{
			printf("Image index %d is NULL!\n", animation[self->currentAnim].frameID[self->currentFrame]);
		}

		self->w = image->w;
		self->h = image->h;
		
		self->offsetX = animation[self->currentAnim].offsetX[self->currentFrame];
		self->offsetY = animation[self->currentAnim].offsetY[self->currentFrame];
	}
	
	else
	{
		image = getSpriteImage(animation[self->currentAnim].frameID[self->currentFrame]);
	}
	
	if (self->face == LEFT)
	{
		if (self->parent == NULL)
		{
			x = self->x - mapStartX();
			y = self->y - mapStartY();
		}
		
		else
		{
			x = self->x - mapStartX() + self->parent->w - self->w - self->offsetX;
			y = self->y - mapStartY() + self->offsetY;
		}
		
		drawFlippedImage(image, x, y);
	}
	
	else
	{
		if (self->parent == NULL)
		{
			x = self->x - mapStartX();
			y = self->y - mapStartY();
		}
		
		else
		{
			x = self->x - mapStartX() + self->offsetX;
			y = self->y - mapStartY() + self->offsetY;
		}
		
		drawImage(image, x, y);
	}
}

void setEntityAnimation(Entity *e, int animationID)
{
	SDL_Surface *image;
	
	if (e->currentAnim != e->animation[animationID])
	{
		e->currentAnim = e->animation[animationID];
		
		e->currentFrame = 0;
		e->frameTimer = animation[e->currentAnim].frameTimer[0];
		
		image = getSpriteImage(animation[e->currentAnim].frameID[0]);
		
		if (image == NULL)
		{
			printf("Image index %d is NULL!\n", animation[e->currentAnim].frameID[0]);
		}
		
		e->w = image->w;
		e->h = image->h;
		
		e->offsetX = animation[e->currentAnim].offsetX[e->currentFrame];
		e->offsetY = animation[e->currentAnim].offsetY[e->currentFrame];
	}
}
