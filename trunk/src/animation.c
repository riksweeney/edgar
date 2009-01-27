#include "headers.h"

static Animation animation[MAX_ANIMATIONS];
extern Entity *self;
extern Map map;

static int animationID = -1;

void loadAnimationData(char *filename, int *spriteIndex, int *animationIndex)
{
	char frameName[255];
	int i;
	FILE *fp;

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Failed to open animation file: %s\n", filename);

		exit(1);
	}

	printf("Loading animation data from %s\n", filename);

	for (i=0;i<MAX_ANIMATION_TYPES;i++)
	{
		animationIndex[i] = -1;
	}

	while (!feof(fp))
	{
		i = fscanf(fp, "%s", frameName);

		if (i == -1 || frameName[0] == '#' || frameName[0] == '\n')
		{
			continue;
		}

		if (strcmpignorecase(frameName, "NAME") == 0)
		{
			if (animationID != -1)
			{
				if (animation[animationID].frameCount == 0)
				{
					printf("Animation %d was created with 0 frames\n", animationID);

					exit(1);
				}
			}

			fscanf(fp, "%s", frameName);

			animationID++;

			if (animationID == MAX_ANIMATIONS)
			{
				printf("Ran out of space for animations\n");

				abort();
			}

			if (strcmpignorecase(frameName, "STAND") == 0)
			{
				animationIndex[STAND] = animationID;
			}

			else if (strcmpignorecase(frameName, "WALK") == 0)
			{
				animationIndex[WALK] = animationID;
			}

			else if (strcmpignorecase(frameName, "JUMP") == 0)
			{
				animationIndex[JUMP] = animationID;
			}

			else if (strcmpignorecase(frameName, "PAIN") == 0)
			{
				animationIndex[PAIN] = animationID;
			}

			else if (strcmpignorecase(frameName, "DIE") == 0)
			{
				animationIndex[DIE] = animationID;
			}

			else if (strcmpignorecase(frameName, "ATTACK_1") == 0)
			{
				animationIndex[ATTACK_1] = animationID;
			}

			else if (strcmpignorecase(frameName, "ATTACK_2") == 0)
			{
				animationIndex[ATTACK_2] = animationID;
			}

			else if (strcmpignorecase(frameName, "ATTACK_3") == 0)
			{
				animationIndex[ATTACK_3] = animationID;
			}

			else if (strcmpignorecase(frameName, "ATTACK_4") == 0)
			{
				animationIndex[ATTACK_4] = animationID;
			}

			else if (strcmpignorecase(frameName, "ATTACK_5") == 0)
			{
				animationIndex[ATTACK_5] = animationID;
			}

			else
			{
				printf("Unknown animation %s\n", frameName);

				exit(1);
			}
		}

		else if (strcmpignorecase(frameName, "FRAMES") == 0)
		{
			fscanf(fp, "%d", &animation[animationID].frameCount);

			/* Allocate space for the frame timer */

			animation[animationID].frameTimer = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].frameTimer == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);

				exit(1);
			}

			/* Allocate space for the frame timer */

			animation[animationID].frameID = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].frameID == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);

				exit(1);
			}

			/* Allocate space for the offsets */

			animation[animationID].offsetX = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].offsetX == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);

				exit(1);
			}

			animation[animationID].offsetY = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].offsetY == NULL)
			{
				printf("Ran out of memory when creating the animation for %s\n", filename);

				exit(1);
			}

			/* Now load up each frame */

			for (i=0;i<animation[animationID].frameCount;i++)
			{
				fscanf(fp, "%d %d %d %d", &animation[animationID].frameID[i], &animation[animationID].frameTimer[i], &animation[animationID].offsetX[i], &animation[animationID].offsetY[i]);

				/* Reassign the Animation frame to the appropriate Sprite index */

				animation[animationID].frameID[i] = spriteIndex[animation[animationID].frameID[i]];
			}
		}
	}

	if (animation[animationID].frameCount == 0)
	{
		printf("Animation %d was created with 0 frames\n", animationID);

		exit(1);
	}

	fclose(fp);
}

static void freeAnimation(Animation *anim)
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

	animationID = -1;
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
	void (*callback)(void);

	self->frameTimer--;

	if (self->frameTimer <= 0)
	{
		self->currentFrame++;

		if (self->currentFrame >= animation[self->currentAnim].frameCount)
		{
			self->currentFrame = 0;

			if (self->animationCallback != NULL)
			{
				callback = self->animationCallback;

				self->animationCallback = NULL;

				callback();
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

		if (e->currentAnim == -1)
		{
			printf("Animation not set for %s index %d\n", e->name, animationID);

			abort();
		}

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
