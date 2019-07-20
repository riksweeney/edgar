/*
Copyright (C) 2009-2019 Parallel Realities
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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../graphics/graphics.h"
#include "../system/error.h"

extern Game game;

static TextureCache textureCacheHead;

static int enabled;

void initTextureCache()
{
	textureCacheHead.next = NULL;

	enabled = TRUE;
}

Texture *getTextureFromCache(char *name)
{
	TextureCache *textureCache;

	if (enabled == FALSE)
	{
		return NULL;
	}

	textureCache = textureCacheHead.next;

	while (textureCache != NULL)
	{
		if (strcmpignorecase(textureCache->name, name) == 0)
		{
			textureCache->expiry = textureCache->expiry == -1 ? -1 : game.frames + TEXTURE_CACHE_TIME;

			return textureCache->texture;
		}

		textureCache = textureCache->next;
	}

	return NULL;
}

void addTextureToCache(char *name, Texture *texture, int doNotExpire)
{
	TextureCache *textureCache, *current;

	if (enabled == FALSE)
	{
		return;
	}

	current = &textureCacheHead;

	while (current->next != NULL)
	{
		current = current->next;
	}

	textureCache = malloc(sizeof(TextureCache));

	if (textureCache == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for a texture cache", sizeof(TextureCache));
	}

	STRNCPY(textureCache->name, name, MAX_MESSAGE_LENGTH);

	textureCache->texture = texture;

	textureCache->expiry = doNotExpire == TRUE ? -1 : game.frames + TEXTURE_CACHE_TIME;

	textureCache->next = NULL;

	current->next = textureCache;
}

void checkTextureCache()
{
	int removed = 0;
	TextureCache *prev, *t1, *t2;

	prev = &textureCacheHead;

	for (t1=textureCacheHead.next;t1!=NULL;t1=t2)
	{
		t2 = t1->next;

		if (t1->expiry != -1 && t1->expiry < game.frames)
		{
			prev->next = t2;

			removed++;
			
            #if DEV == 1
                printf("Expiring %s\n", t1->name);
            #endif

			destroyTexture(t1->texture);

			free(t1);

			t1 = NULL;
		}

		else
		{
			prev = prev->next;
		}
	}

	#if DEV == 1
		if (removed > 0)
		{
			printf("Flushed %d expired textures\n", removed);
		}
	#endif
}

void freeTextureCache()
{
	TextureCache *p, *q;

	for (p=textureCacheHead.next;p!=NULL;p=q)
	{
		destroyTexture(p->texture);

		q = p->next;

		free(p);
	}
}
