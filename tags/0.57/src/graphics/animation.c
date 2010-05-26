/*
Copyright (C) 2009-2010 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "animation.h"
#include "graphics.h"
#include "sprites.h"
#include "../map.h"
#include "../collisions.h"
#include "../system/pak.h"
#include "../system/error.h"

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
					{BLOCK, "BLOCK"},
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
	char *frameName, *line, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int i;

	savePtr1 = NULL;

	buffer = loadFileFromPak(filename);

	for (i=0;i<MAX_ANIMATION_TYPES;i++)
	{
		animationIndex[i] = -1;
	}

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	while (line != NULL)
	{
		frameName = strtok_r(line, " ", &savePtr2);

		if (frameName[0] == '#' || frameName[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr1);

			continue;
		}

		if (strcmpignorecase(frameName, "NAME") == 0)
		{
			if (animationID != -1)
			{
				if (animation[animationID].frameCount == 0)
				{
					showErrorAndExit("Animation %d from file %s was created with 0 frames\n", animationID, filename);
				}
			}

			frameName = strtok_r(NULL, " ", &savePtr2);

			animationID++;

			if (animationID == MAX_ANIMATIONS)
			{
				showErrorAndExit("Ran out of space for animations\n");
			}

			animationIndex[getAnimationTypeByName(frameName)] = animationID;
		}

		else if (strcmpignorecase(frameName, "FRAMES") == 0)
		{
			frameName = strtok_r(NULL, " ", &savePtr2);

			animation[animationID].frameCount = atoi(frameName);

			/* Allocate space for the frame timer */

			animation[animationID].frameTimer = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].frameTimer == NULL)
			{
				showErrorAndExit("Ran out of memory when creating the animation for %s\n", filename);
			}

			/* Allocate space for the frame timer */

			animation[animationID].frameID = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].frameID == NULL)
			{
				showErrorAndExit("Ran out of memory when creating the animation for %s\n", filename);
			}

			/* Allocate space for the offsets */

			animation[animationID].offsetX = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].offsetX == NULL)
			{
				showErrorAndExit("Ran out of memory when creating the animation for %s\n", filename);
			}

			animation[animationID].offsetY = (int *)malloc(animation[animationID].frameCount * sizeof(int));

			if (animation[animationID].offsetY == NULL)
			{
				showErrorAndExit("Ran out of memory when creating the animation for %s\n", filename);
			}

			/* Now load up each frame */

			for (i=0;i<animation[animationID].frameCount;i++)
			{
				line = strtok_r(NULL, "\n", &savePtr1);

				frameName = strtok_r(line, " ", &savePtr2);

				animation[animationID].frameID[i] = atoi(frameName);

				frameName = strtok_r(NULL, " ", &savePtr2);

				animation[animationID].frameTimer[i] = atoi(frameName);

				frameName = strtok_r(NULL, " ", &savePtr2);

				animation[animationID].offsetX[i] = atoi(frameName);

				frameName = strtok_r(NULL, "\0", &savePtr2);

				animation[animationID].offsetY[i] = atoi(frameName);

				/* Reassign the Animation frame to the appropriate Sprite index */

				if (spriteIndex[animation[animationID].frameID[i]] == -1)
				{
					showErrorAndExit("Invalid sprite at animation index %d in file %s", animation[animationID].frameID[i], filename);
				}

				animation[animationID].frameID[i] = spriteIndex[animation[animationID].frameID[i]];
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (animation[animationID].frameCount == 0)
	{
		showErrorAndExit("Animation %d from file %s was created with 0 frames", animationID, filename);
	}

	free(buffer);
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
			showErrorAndExit("Image index %d is NULL!\n", animation[e->currentAnim].frameID[e->currentFrame]);
		}

		e->w = sprite->image->w;
		e->h = sprite->image->h;

		e->box = sprite->box;
	}

	else
	{
		sprite = getSprite(animation[e->currentAnim].frameID[e->currentFrame]);
	}

	if (center == 1)
	{
		drawImage(sprite->image, x + (w - sprite->image->w) / 2, y + (h - sprite->image->h) / 2, FALSE, e->alpha);
	}

	else
	{
		drawImage(sprite->image, x, y, FALSE, e->alpha);
	}
}

int drawLoopingAnimationToMap()
{
	int x, y, drawn;
	int startX, startY;
	Sprite *sprite;
	void (*callback)(void);

	drawn = FALSE;

	startX = getMapStartX();
	startY = getMapStartY();

	self->frameTimer -= 1 * fabs(self->frameSpeed);

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
					return drawn;
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
					return drawn;
				}
			}
		}

		self->frameTimer = animation[self->currentAnim].frameTimer[self->currentFrame];

		if (self->face == LEFT)
		{
			sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame] + 1);
		}

		else
		{
			sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame]);
		}

		if (sprite->image == NULL)
		{
			showErrorAndExit("Image index %d is NULL!\n", animation[self->currentAnim].frameID[self->currentFrame]);
		}

		self->w = sprite->image->w;
		self->h = sprite->image->h;

		self->offsetX = animation[self->currentAnim].offsetX[self->currentFrame];
		self->offsetY = animation[self->currentAnim].offsetY[self->currentFrame];

		self->box = sprite->box;
	}

	else
	{
		if (self->face == LEFT)
		{
			sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame] + 1);
		}

		else
		{
			sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame]);
		}
	}

	if (self->alpha <= 0)
	{
		return FALSE;
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
	}

	if (collision(x, y, sprite->image->w, sprite->image->h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
	{
		drawn = TRUE;

		drawImage(sprite->image, x, y, (self->flags & FLASH) ? TRUE : FALSE, self->alpha);

		/*drawHitBox(x + self->w - self->box.w - self->box.x, y + self->box.y, self->box.w, self->box.h);*/
	}

	return drawn;
}

int drawSpriteToMap()
{
	int x, y, drawn;
	int startX, startY;
	Sprite *sprite;

	drawn = FALSE;

	startX = getMapStartX();
	startY = getMapStartY();

	if (self->face == LEFT)
	{
		sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame] + 1);
	}

	else
	{
		sprite = getSprite(animation[self->currentAnim].frameID[self->currentFrame]);
	}

	if (self->alpha <= 0)
	{
		return FALSE;
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
	}

	if (collision(x, y, sprite->image->w, sprite->image->h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
	{
		drawn = TRUE;

		drawImage(sprite->image, x, y, (self->flags & FLASH) ? TRUE : FALSE, self->alpha);

		/*drawHitBox(x + self->w - self->box.w - self->box.x, y + self->box.y, self->box.w, self->box.h);*/
	}

	return drawn;
}

void drawSprite(Entity *e, int x, int y, int w, int h, int center)
{
	Sprite *sprite;

	sprite = getSprite(animation[e->currentAnim].frameID[e->currentFrame]);

	if (center == 1)
	{
		drawImage(sprite->image, x + (w - sprite->image->w) / 2, y + (h - sprite->image->h) / 2, FALSE, e->alpha);
	}

	else
	{
		drawImage(sprite->image, x, y, FALSE, e->alpha);
	}
}

int drawLineDefToMap()
{
	drawBoxToMap(self->x, self->y, self->w, self->h, 255, 0, 0);

	return TRUE;
}

void setEntityAnimation(Entity *e, int animationID)
{
	int previousRightEdge, previousAnim, newRightEdge;
	Sprite *sprite;

	if (e->inUse == FALSE)
	{
		return;
	}

	previousAnim = e->currentAnim;

	if (previousAnim == -1)
	{
		previousRightEdge = -1;
	}

	else
	{
		previousRightEdge = e->x + e->w;
	}

	if (e->currentAnim != e->animation[animationID])
	{
		e->currentAnim = e->animation[animationID];

		if (e->currentAnim == -1)
		{
			showErrorAndExit("Animation %s not set for %s", getAnimationTypeByID(animationID), e->name);
		}

		e->currentFrame = (e->frameSpeed >= 0 ? 0 : animation[e->currentAnim].frameCount - 1);
		e->frameTimer = animation[e->currentAnim].frameTimer[e->frameSpeed >= 0 ? 0 : animation[e->currentAnim].frameCount - 1];

		sprite = getSprite(animation[e->currentAnim].frameID[e->frameSpeed >= 0 ? 0 : animation[e->currentAnim].frameCount - 1]);

		if (sprite->image == NULL)
		{
			showErrorAndExit("Image index %d for %s is NULL!", animation[e->currentAnim].frameID[0], e->name);
		}

		e->w = sprite->image->w;
		e->h = sprite->image->h;

		e->offsetX = animation[e->currentAnim].offsetX[e->currentFrame];
		e->offsetY = animation[e->currentAnim].offsetY[e->currentFrame];

		e->box = sprite->box;

		/* Align the right edge to stop it looking bad */

		if (previousAnim != -1)
		{
			if (e->face == LEFT)
			{
				newRightEdge = e->x + e->w;

				e->x += (previousRightEdge - newRightEdge);
			}
		}
	}
}

int hasEntityAnimation(Entity *e, int animationID)
{
	return e->animation[animationID] == -1 ? FALSE : TRUE;
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

	showErrorAndExit("Unknown animation %s", name);

	return 0;
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

	showErrorAndExit("Unknown animation index %d", id);

	return 0;
}

int getAnimationTypeAtIndex(Entity *e)
{
	int i;

	for (i=0;i<MAX_ANIMATION_TYPES;i++)
	{
		if (e->currentAnim == e->animation[i])
		{
			return i;
		}
	}

	showErrorAndExit("Failed to find animation at index %d", e->currentAnim);

	return 0;
}

void setFrameData(Entity *e)
{
	Sprite *sprite;

	if (e->inUse == FALSE)
	{
		return;
	}

	sprite = getSprite(animation[e->currentAnim].frameID[e->currentFrame]);

	e->w = sprite->image->w;
	e->h = sprite->image->h;

	e->offsetX = animation[e->currentAnim].offsetX[e->currentFrame];
	e->offsetY = animation[e->currentAnim].offsetY[e->currentFrame];

	e->box = sprite->box;
}

Sprite *getCurrentSprite(Entity *e)
{
	if (self->face == LEFT)
	{
		return getSprite(animation[self->currentAnim].frameID[self->currentFrame] + 1);
	}

	return getSprite(animation[self->currentAnim].frameID[self->currentFrame]);
}

int getFrameCount(Entity *e)
{
	return animation[e->currentAnim].frameCount;
}
