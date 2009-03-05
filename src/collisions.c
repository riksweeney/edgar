#include "headers.h"

#include "collisions.h"
#include "map.h"

extern Entity entity[MAX_ENTITIES], *self;
extern Entity player, playerShield, playerWeapon;

void doCollisions()
{
	int i, x, y, w, h, hit;

	hit = 0;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE)
		{
			if (playerWeapon.inUse == TRUE && (playerWeapon.flags & ATTACKING) && !(playerWeapon.flags & ATTACK_SUCCESS))
			{
				x = playerWeapon.x + playerWeapon.offsetX * (player.face == LEFT ? -1 : 1);
				y = playerWeapon.y + playerWeapon.offsetY * (player.face == LEFT ? -1 : 1);
				w = playerWeapon.w;
				h = playerWeapon.h;

				if (collision(entity[i].x, entity[i].y, entity[i].w, entity[i].h, x, y, w, h) == 1)
				{
					if (entity[i].touch != NULL)
					{
						self = &entity[i];

						self->touch(&playerWeapon);

						hit = TRUE;
					}
				}
			}

			if (entity[i].touch != NULL)
			{
				if (collision(entity[i].x, entity[i].y, entity[i].w, entity[i].h, player.x, player.y, player.w, player.h) == 1)
				{
					self = &entity[i];

					self->touch(&player);
				}

				checkEntityToEntity(&entity[i]);
			}
		}
	}

	if (hit == 1)
	{
		playerWeapon.flags |= ATTACK_SUCCESS;
	}
}

void checkEntityToEntity(Entity *e)
{
	int i;
	Entity *temp;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (e == &entity[i] || entity[i].inUse == FALSE || (e->type == ENEMY && entity[i].type == ENEMY)
			|| entity[i].touch == NULL || e->type == LEVEL_EXIT || e->type == LINE_DEF || e->type == SPAWNER
			|| entity[i].type == SAVE_POINT || (e->type == PROJECTILE && entity[i].type == PROJECTILE))
		{
			continue;
		}

		if (collision(e->x, e->y, e->w, e->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
		{
			temp = self;

			self = &entity[i];

			self->touch(e);

			self = temp;
		}
	}
}

Entity *isSpaceEmpty(Entity *e)
{
	int i;

	if (player.inUse == TRUE && collision(e->x, e->y, e->w, e->h, player.x, player.y, player.w, player.h) == 1)
	{
		return &player;
	}

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && collision(e->x, e->y, e->w, e->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
		{
			return &entity[i];
		}
	}

	return NULL;
}

void checkToMap(Entity *e)
{
	int i, x1, x2, y1, y2;
	int topLeft, topRight, bottomLeft, bottomRight;

	/* Remove the entity from the ground */

	e->flags &= ~ON_GROUND;

	/* Set environment to air */

	e->environment = AIR;

	/* Test the horizontal movement first */

	i = e->h > TILE_SIZE ? TILE_SIZE : e->h;

	for (;;)
	{
		x1 = (e->x + e->dirX) / TILE_SIZE;
		x2 = (e->x + e->dirX + e->w - 1) / TILE_SIZE;

		y1 = (e->y) / TILE_SIZE;
		y2 = (e->y + i - 1) / TILE_SIZE;

		if (x1 >= 0 && x2 < MAX_MAP_X && y1 >= 0 && y2 < MAX_MAP_Y)
		{
			topLeft     = mapTileAt(x1, y1);
			topRight    = mapTileAt(x2, y1);
			bottomLeft  = mapTileAt(x1, y2);
			bottomRight = mapTileAt(x2, y2);

			if (e->dirX > 0)
			{
				/* Trying to move right */

				if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					if (i == e->h)
					{
						if (!(e->flags & FLY))
						{
							e->y -= e->dirX;

							e->dirY = 0;

							e->flags |= ON_GROUND;
						}

						if (e->type == PROJECTILE)
						{
							e->inUse = FALSE;

							return;
						}
					}
				}

				else if (bottomRight >= SLOPE_DOWN_START && bottomRight <= SLOPE_DOWN_END)
				{

				}

				else if ((topRight >= JUMP_THROUGH_TILE_START && topRight <= JUMP_THROUGH_TILE_END) ||
					((bottomRight >= JUMP_THROUGH_TILE_START && bottomRight <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topRight != BLANK_TILE && topRight < BACKGROUND_TILE_START) || (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->x = x2 * TILE_SIZE;

					e->x -= e->w;

					e->dirX = 0;

					if ((e->flags & GRABBING) && e->target != NULL)
					{
						e->target->dirX = 0;
					}

					if (e->type == PROJECTILE)
					{
						e->inUse = FALSE;

						return;
					}
				}
			}

			else if (e->dirX < 0)
			{
				/* Trying to move left */

				if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					if (i == e->h)
					{
						if (!(e->flags & FLY))
						{
							e->y += e->dirX;

							e->dirY = 0;

							e->flags |= ON_GROUND;
						}
					}
				}

				else if (bottomLeft >= SLOPE_UP_START && bottomLeft <= SLOPE_UP_END)
				{

				}

				else if ((topLeft >= JUMP_THROUGH_TILE_START && topLeft <= JUMP_THROUGH_TILE_END) ||
					((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topLeft != BLANK_TILE && topLeft < BACKGROUND_TILE_START) || (bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->x = (x1 + 1) * TILE_SIZE;

					e->dirX = 0;

					if ((e->flags & GRABBING) && e->target != NULL)
					{
						e->target->dirX = 0;
					}

					if (e->type == PROJECTILE)
					{
						e->inUse = FALSE;

						return;
					}
				}
			}
		}

		/* Exit this loop if we have tested all of the body */

		if (i == e->h || e->h <= TILE_SIZE)
		{
			break;
		}

		/* Test the next block */

		i += TILE_SIZE;

		if (i > e->h)
		{
			i = e->h;
		}
	}

	/* Now test the vertical movement */

	i = e->w > TILE_SIZE ? TILE_SIZE : e->w;

	for (;;)
	{
		x1 = (e->x) / TILE_SIZE;
		x2 = (e->x + i - 1) / TILE_SIZE;

		y1 = (e->y + e->dirY) / TILE_SIZE;
		y2 = (e->y + e->dirY + e->h) / TILE_SIZE;

		if (x1 >= 0 && x2 < MAX_MAP_X && y1 >= 0 && y2 < MAX_MAP_Y)
		{
			topLeft     = mapTileAt(x1, y1);
			topRight    = mapTileAt(x2, y1);
			bottomLeft  = mapTileAt(x1, y2);
			bottomRight = mapTileAt(x2, y2);

			if (e->dirY > 0)
			{
				/* Trying to move down */

				if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					if (i == e->w)
					{
						e->y = (y2 + 1) * TILE_SIZE;
						e->y -= e->h;

						e->y -= (((int)e->x + e->w - 1) % TILE_SIZE) + 1;

						e->dirY = 0;

						e->flags |= ON_GROUND;
					}

					if (e->type == PROJECTILE)
					{
						e->inUse = FALSE;

						return;
					}
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					if (i == e->w)
					{
						e->y = (y2 + 1) * TILE_SIZE;
						e->y -= e->h;

						e->y -= TILE_SIZE - ((int)e->x) % TILE_SIZE;

						e->dirY = 0;

						e->flags |= ON_GROUND;
					}

					if (e->type == PROJECTILE)
					{
						e->inUse = FALSE;

						return;
					}
				}

				else if ((bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START) || (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->y = y2 * TILE_SIZE;
					e->y -= e->h;

					e->dirY = 0;

					e->flags |= ON_GROUND;

					if (e->type == PROJECTILE)
					{
						e->inUse = FALSE;

						return;
					}
				}
			}

			else if (e->dirY < 0)
			{
				/* Trying to move up */

				if ((topLeft >= JUMP_THROUGH_TILE_START && topLeft <= JUMP_THROUGH_TILE_END) ||
					((topRight >= JUMP_THROUGH_TILE_START && topRight <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topLeft != BLANK_TILE && topLeft < BACKGROUND_TILE_START) || (topRight != BLANK_TILE && topRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->y = (y1 + 1) * TILE_SIZE;

					e->dirY = 0;

					if (e->type == PROJECTILE)
					{
						e->inUse = FALSE;

						return;
					}
				}
			}
		}

		if (i == e->w || e->w <= TILE_SIZE)
		{
			break;
		}

		i += TILE_SIZE;

		if (i > e->w)
		{
			i = e->w;
		}
	}

	/* Now apply the movement */

	e->x += e->dirX;
	e->y += e->dirY;

	x1 = e->type == PLAYER ? getMinMapX() : 0;
	y1 = e->type == PLAYER ? getMinMapY() : 0;

	if (e->x < x1)
	{
		e->x = x1;

		e->dirX = 0;

		if ((e->flags & GRABBING) && e->target != NULL)
		{
			e->target->dirX = 0;
		}

		if (e->type == PROJECTILE)
		{
			e->inUse = FALSE;
		}

	}

	else if (e->x + e->w >= maxMapX())
	{
		e->x = maxMapX() - e->w - 1;

		e->dirX = 0;

		if ((e->flags & GRABBING) && e->target != NULL)
		{
			e->target->dirX = 0;
		}

		if (e->type == PROJECTILE)
		{
			e->inUse = FALSE;
		}
	}

	if (e->y > maxMapY())
	{
		e->fallout();
	}

	if ((topLeft >= WATER_TILE_START && topLeft <= WATER_TILE_END) ||
		(bottomLeft >= WATER_TILE_START && bottomLeft <= WATER_TILE_END) ||
		(topRight >= WATER_TILE_START && topRight <= WATER_TILE_END) ||
		(bottomRight >= WATER_TILE_START && bottomRight <= WATER_TILE_END))
	{
		e->environment = WATER;
	}

	else if ((topLeft >= LAVA_TILE_START && topLeft <= LAVA_TILE_END) ||
		(bottomLeft >= LAVA_TILE_START && bottomLeft <= LAVA_TILE_END) ||
		(topRight >= LAVA_TILE_START && topRight <= LAVA_TILE_END) ||
		(bottomRight >= LAVA_TILE_START && bottomRight <= LAVA_TILE_END))
	{
		e->environment = LAVA;
	}
}

int isAtEdge(Entity *e)
{
	int x = e->x + (e->dirX > 0 ? e->w : 0);
	int y = e->y + e->h - 1;
	int i;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	/* Return immediately if the tile isn't blank */

	if (!(e->flags & ON_GROUND) || (mapTileAt(x, y + 1) != BLANK_TILE && mapTileAt(x, y + 1) < BACKGROUND_TILE_START))
	{
		return FALSE;
	}

	/* There might still be Entities that can be walked on */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (e != &entity[i] && (entity[i].flags & PUSHABLE))
		{
			if (collision(e->x + (e->face == LEFT ? -5 : 5), e->y + 5, e->w, e->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
			{
				if (collision(e->x + (e->face == LEFT ? -5 : 5), e->y, e->w, e->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 0)
				{
					return FALSE;
				}

				return TRUE;
			}
		}
	}

	return TRUE;
}

int isValidOnMap(Entity *e)
{
	int i, x1, x2, y1, y2;

	i = e->w > TILE_SIZE ? TILE_SIZE : e->w;

	for (;;)
	{
		x1 = (e->x) / TILE_SIZE;
		x2 = (e->x + i - 1) / TILE_SIZE;

		y1 = (e->y) / TILE_SIZE;
		y2 = (e->y + e->h - 1) / TILE_SIZE;

		if ((mapTileAt(x1, y1) < BACKGROUND_TILE_START && mapTileAt(x1, y1) > BLANK_TILE) ||
			(mapTileAt(x2, y1) < BACKGROUND_TILE_START && mapTileAt(x2, y1) > BLANK_TILE) ||
			(mapTileAt(x1, y2) < BACKGROUND_TILE_START && mapTileAt(x1, y2) > BLANK_TILE) ||
			(mapTileAt(x2, y2) < BACKGROUND_TILE_START && mapTileAt(x2, y2) > BLANK_TILE))
		{
			return FALSE;
		}

		if (i == e->w)
		{
			break;
		}

		i += e->w;

		if (i > e->w)
		{
			i = e->w;
		}
	}

	i = e->h > TILE_SIZE ? TILE_SIZE : e->h;

	for (;;)
	{
		x1 = (e->x) / TILE_SIZE;
		x2 = (e->x + e->w - 1) / TILE_SIZE;

		y1 = (e->y) / TILE_SIZE;
		y2 = (e->y + i - 1) / TILE_SIZE;

		if ((mapTileAt(x1, y1) < BACKGROUND_TILE_START && mapTileAt(x1, y1) > BLANK_TILE) ||
			(mapTileAt(x2, y1) < BACKGROUND_TILE_START && mapTileAt(x2, y1) > BLANK_TILE) ||
			(mapTileAt(x1, y2) < BACKGROUND_TILE_START && mapTileAt(x1, y2) > BLANK_TILE) ||
			(mapTileAt(x2, y2) < BACKGROUND_TILE_START && mapTileAt(x2, y2) > BLANK_TILE))
		{
			return FALSE;
		}

		if (i == e->h)
		{
			break;
		}

		i += e->h;

		if (i > e->h)
		{
			i = e->h;
		}
	}

	return TRUE;
}

/* Very standard 2D collision detection routine */

int collision(int x0, int y0, int w0, int h0, int x2, int y2, int w1, int h1)
{
	int x1 = x0 + w0 - 1;
	int y1 = y0 + h0 - 1;

	int x3 = x2 + w1 - 1;
	int y3 = y2 + h1 - 1;

	return !(x1<x2 || x3<x0 || y1<y2 || y3<y0);
}
