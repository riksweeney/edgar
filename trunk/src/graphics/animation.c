#include "../headers.h"

#include "animation.h"
#include "graphics.h"
#include "sprites.h"
#include "../map.h"
#include "../collisions.h"

static Animation animation[MAX_ANIMATIONS];
extern Entity *self;
extern Map map;

static char *getAnimationTypeByID(int);

static int animationID = -1;
static Type type[] = {
					{STAND, "STAND"},
					{WALK, "WALK"},
					{JUMP, "JUMP"},
					{PAIN, "PAIN"},
					{DIE, "DIE"},
					{ATTACK_1, "ATTACK_1"},
					{ATTACK_2, "ATTACK_2"},
					{ATTACK_3, "ATTACK_3"},
					{ATTACK_4, "ATTACK_4"},
					{ATTACK_5, "ATTACK_5"},
					{CUSTOM_1, "CUSTOM_1"},
					{CUSTOM_2, "CUSTOM_2"},
					{CUSTOM_3, "CUSTOM_3"},
					{CUSTOM_4, "CUSTOM_4"},
					{CUSTOM_5, "CUSTOM_5"},
					{TAUNT, "TAUNT"}
					};

void loadAnimationData(char *filename, int *spriteIndex, int *animationIndex)
{
	char frameName[MAX_MESSAGE_LENGTH];
	int i;
	FILE *fp;

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Failed to open animation file: %s\n", filename);

		exit(1);
	}

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
					printf("Animation %d from file %s was created with 0 frames\n", animationID, filename);

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

			animationIndex[getAnimationTypeByName(frameName)] = animationID;
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

				if (spriteIndex[animation[animationID].frameID[i]] == -1)
				{
					printf("Invalid sprite at animation index %d in file \n", animation[animationID].frameID[i], filename);

					exit(1);
				}

				animation[animationID].frameID[i] = spriteIndex[animation[animationID].frameID[i]];
			}
		}
	}

	if (animation[animationID].frameCount == 0)
	{
		printf("Animation %d from file %s was created with 0 frames\n", animationID, filename);

		exit(1);
	}

	fclose(fp);
}

static void freeAnimation(Animation *anim)
{
	if (anim->frameTimer != NULL)
	{
		free(anim->frameTimer);

		anim->frameTimer = NULL;
	}

	if (anim->frameID != NULL)
	{
		free(anim->frameID);

		anim->frameID = NULL;
	}

	if (anim->offsetX != NULL)
	{
		free(anim->offsetX);

		anim->offsetX = NULL;
	}

	if (anim->offsetY != NULL)
	{
		free(anim->offsetY);

		anim->offsetY = NULL;
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
	Sprite *sprite;

	e->frameTimer--;

	if (e->frameTimer <= 0)
	{
		e->currentFrame++;

		if (e->currentFrame >= animation[e->currentAnim].frameCount)
		{
			e->currentFrame = 0;
		}

		e->frameTimer = animation[e->currentAnim].frameTimer[e->currentFrame];

		sprite = getSprite(animation[e->currentAnim].frameID[e->currentFrame]);

		if (sprite->image == NULL)
		{
			printf("Image index %d is NULL!\n", animation[e->currentAnim].frameID[e->currentFrame]);
		}

		e->w = sprite->image->w;
		e->h = sprite->image->h;
	}

	else
	{
		sprite = getSprite(animation[e->currentAnim].frameID[e->currentFrame]);
	}

	if (center == 1)
	{
		drawImage(sprite->image, x + (w - sprite->image->w) / 2, y + (h - sprite->image->h) / 2, FALSE);
	}

	else
	{
		drawImage(sprite->image, x, y, FALSE);
	}
}

void drawLoopingAnimationToMap()
{
	int x, y;
	int startX, startY;
	Sprite *sprite;
	void (*callback)(void);

	startX = getMapStartX();
	startY = getMapStartY();

	self->frameTimer -= 1 * abs(self->frameSpeed);

	if (self->frameTimer <= 0)
	{
		self->currentFrame += self->frameSpeed > 0 ? 1 : -1;

		if (self->currentFrame >= animation[self->currentAnim].frameCount)
		{
			self->currentFrame = self->animationCallback == NULL ? 0 : animation[self->currentAnim].frameCount - 1;

			if (self->animationCallback != NULL)
			{
				callback = self->animationCallback;

				self->animationCallback = NULL;

				callback();

				if (self->inUse == FALSE)
				{
					return;
				}
			}
		}

		else if (self->currentFrame < 0)
		{
			self->currentFrame = self->animationCallback == NULL ? animation[self->currentAnim].frameCount - 1 : 0;

			if (self->animationCallback != NULL)
			{
				callback = self->animationCallback;

				self->animationCallback = NULL;

				callback();

				if (self->inUse == FALSE)
				{
					return;
				}
			}
		}

		self->frameTimer = animation[self->currentAnim].frameTimer[self->currentFrame];

		sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame]);

		if (sprite->image == NULL)
		{
			printf("Image index %d is NULL!\n", animation[self->currentAnim].frameID[self->currentFrame]);
		}

		self->w = sprite->image->w;
		self->h = sprite->image->h;

		self->offsetX = animation[self->currentAnim].offsetX[self->currentFrame];
		self->offsetY = animation[self->currentAnim].offsetY[self->currentFrame];
	}

	else
	{
		sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame]);
	}

	if (self->face == LEFT)
	{
		if (self->parent == NULL || self->type == PROJECTILE)
		{
			x = self->x - startX;
			y = self->y - startY;
		}

		else
		{
			x = self->x - startX + self->parent->w - self->w - self->offsetX;
			y = self->y - startY + self->offsetY;
		}

		if (collision(x, y, sprite->image->w, sprite->image->h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
		{
			drawFlippedImage(sprite->image, x, y, (self->flags & FLASH) ? TRUE : FALSE);
		}
	}

	else
	{
		if (self->parent == NULL || self->type == PROJECTILE)
		{
			x = self->x - startX;
			y = self->y - startY;
		}

		else
		{
			x = self->x - startX + self->offsetX;
			y = self->y - startY + self->offsetY;
		}

		if (collision(x, y, sprite->image->w, sprite->image->h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
		{
			drawImage(sprite->image, x, y, (self->flags & FLASH) ? TRUE : FALSE);
		}
	}
}

void drawLineDefToMap()
{
	if (self->flags & NO_DRAW)
	{
		/*return;*/
	}

	drawBoxToMap(self->x, self->y, self->w, self->h, 255, 0, 0);
}

void setEntityAnimation(Entity *e, int animationID)
{
	Sprite *sprite;

	if (e->currentAnim != e->animation[animationID])
	{
		e->currentAnim = e->animation[animationID];

		if (e->currentAnim == -1)
		{
			printf("Animation not set for %s index %s\n", e->name, getAnimationTypeByID(animationID));

			exit(1);
		}

		e->currentFrame = (e->frameSpeed >= 0 ? 0 : animation[e->currentAnim].frameCount - 1);
		e->frameTimer = animation[e->currentAnim].frameTimer[0];

		sprite = getSprite(animation[e->currentAnim].frameID[0]);

		if (sprite->image == NULL)
		{
			printf("Image index %d is NULL!\n", animation[e->currentAnim].frameID[0]);
		}

		e->w = sprite->image->w;
		e->h = sprite->image->h;

		e->offsetX = animation[e->currentAnim].offsetX[e->currentFrame];
		e->offsetY = animation[e->currentAnim].offsetY[e->currentFrame];
	}
}

int getAnimationTypeByName(char *name)
{
	int i;

	for (i=0;i<MAX_ANIMATION_TYPES;i++)
	{
		if (strcmpignorecase(name, type[i].name) == 0)
		{
			return type[i].id;
		}
	}

	printf("Unknown animation %s\n", name);

	exit(1);
}

static char *getAnimationTypeByID(int id)
{
	int i;

	for (i=0;i<MAX_ANIMATION_TYPES;i++)
	{
		if (id == type[i].id)
		{
			return type[i].name;
		}
	}

	printf("Unknown animation index %d\n", id);

	exit(1);
}
