#include "headers.h"

#include "collisions.h"
#include "map.h"

extern Entity entity[MAX_ENTITIES], *self;
extern Entity player, playerShield, playerWeapon;

static Grid grid[GRID_HEIGHT][GRID_WIDTH];

static void addToList(int, int, Entity *);

void initCollisionGrid()
{
	int x, y;

	for (y=0;y<GRID_HEIGHT;y++)
	{
		for (x=0;x<GRID_WIDTH;x++)
		{
			grid[y][x].count = 0;
			grid[y][x].listHead.next = NULL;
		}
	}
}

void addToGrid(Entity *e)
{
	int left, right, top, bottom;
	
	if (e == &playerWeapon)
	{		
		left = (e->x + (e->offsetX * e->face == LEFT ? -1 : 1)) / TILE_SIZE / GRID_SIZE;
		right = (e->x + (e->offsetX * e->face == LEFT ? -1 : 1) + e->w - 1) / TILE_SIZE / GRID_SIZE;
		
		top = (e->y + (e->offsetY * e->face == LEFT ? -1 : 1)) / TILE_SIZE / GRID_SIZE;
		bottom = (e->y + (e->offsetY * e->face == LEFT ? -1 : 1) + e->h - 1) / TILE_SIZE / GRID_SIZE;
	}
	
	else
	{
		left = e->x / TILE_SIZE / GRID_SIZE;
		right = (e->x + e->w - 1) / TILE_SIZE / GRID_SIZE;
	
		top = e->y / TILE_SIZE / GRID_SIZE;
		bottom = (e->y + e->h - 1) / TILE_SIZE / GRID_SIZE;
	}

	addToList(top, left, e);

	if (left != right)
	{
		addToList(top, right, e);
	}

	if (top != bottom)
	{
		addToList(bottom, left, e);
	}

	if (left != right && top != bottom)
	{
		addToList(bottom, right, e);
	}
}

static void addToList(int y, int x, Entity *e)
{
	EntityList *listHead, *list;

	listHead = &grid[y][x].listHead;

	while (listHead->next != NULL)
	{
		listHead = listHead->next;
	}

	list = (EntityList *)malloc(sizeof(EntityList));

	list->entity = e;
	list->next = NULL;

	listHead->next = list;

	grid[y][x].count++;
}

void freeCollisionGrid()
{
	int x, y;
	EntityList *p, *q;

	for (y=0;y<GRID_HEIGHT;y++)
	{
		for (x=0;x<GRID_WIDTH;x++)
		{
			for (p=grid[y][x].listHead.next;p!=NULL;p=q)
			{
				q = p->next;

				free(p);
			}

			grid[y][x].count = 0;
			grid[y][x].listHead.next = NULL;
		}
	}
}

void doCollisions()
{
	int i, j, x, y;
	Entity *e1, *e2, *temp;
	EntityList *list1, *list2;

	for (i=0;i<GRID_HEIGHT;i++)
	{
		for (j=0;j<GRID_WIDTH;j++)
		{
			for (list1=grid[i][j].listHead.next;list1!=NULL;list1=list1->next)
			{
				e1 = list1->entity;

				if (e1->inUse == TRUE)
				{
					for (list2=grid[i][j].listHead.next;list2!=NULL;list2=list2->next)
					{
						e2 = list2->entity;

						if (e1 != e2 && e2->inUse == TRUE && e2->touch != NULL)
						{
							if (e1->type == ENEMY && e2->type == ENEMY)
							{
								continue;
							}
							
							if ((e1 == &player && e2 == &playerWeapon) || (e1 == &playerWeapon && e2 == &player))
							{
								continue;
							}
							/*
							if (e1 == &player)
							{
								if (playerWeapon.inUse == TRUE && (playerWeapon.flags & ATTACKING))
								{
									if (e2 != &playerShield && e2 != &player)
									{
										x = playerWeapon.x + playerWeapon.offsetX * (player.face == LEFT ? -1 : 1);
										y = playerWeapon.y + playerWeapon.offsetY * (player.face == LEFT ? -1 : 1);
										w = playerWeapon.w;
										h = playerWeapon.h;
	
										if (collision(e2->x, e2->y, e2->w, e2->h, x, y, w, h) == TRUE)
										{
											temp = self;
	
											self = e2;
	
											self->touch(&playerWeapon);
	
											self = temp;
										}
									}
								}
							}
							*/
							
							x = e1->x;
							y = e1->y;
							
							if (e1 == &playerWeapon)
							{
								x += e1->offsetX * (e1->face == LEFT ? -1 : 1);
								y += e1->offsetY * (e1->face == LEFT ? -1 : 1);
							}
							
							if (collision(x, y, e1->w, e1->h, e2->x, e2->y, e2->w, e2->h) == TRUE)
							{
								temp = self;

								self = e2;

								self->touch(e1);

								self = temp;
							}
						}
					}
				}
			}
		}
	}

	/*
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

				if (entity[i].type != SAVE_POINT && entity[i].type != LINE_DEF && entity[i].type != SPAWNER &&
					entity[i].type != LEVEL_EXIT && entity[i].type != SWITCH)
				{
					checkEntityToEntity(&entity[i]);
				}
			}
		}
	}
	*/
}
/*
void checkEntityToEntity(Entity *e)
{
	int i;
	Entity *temp;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (e == &entity[i] || entity[i].inUse == FALSE || (e->type == ENEMY && entity[i].type == ENEMY) ||
			entity[i].touch == NULL || entity[i].type == LEVEL_EXIT || entity[i].type == LINE_DEF ||
			entity[i].type == SPAWNER || entity[i].type == SAVE_POINT || (e->type == WEAK_WALL && entity[i].type == WEAK_WALL) ||
			(e->type == PROJECTILE && entity[i].type == PROJECTILE) || entity[i].type == SWITCH)
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
*/
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

	x1 = (e->x) / TILE_SIZE;
	x2 = (e->x + e->w - 1) / TILE_SIZE;

	y1 = (e->y) / TILE_SIZE;
	y2 = (e->y + e->h - 1) / TILE_SIZE;

	topLeft     = mapTileAt(x1, y1);
	topRight    = mapTileAt(x2, y1);
	bottomLeft  = mapTileAt(x1, y2);
	bottomRight = mapTileAt(x2, y2);

	if ((topLeft >= LAVA_TILE_START && topLeft <= LAVA_TILE_END) ||
		(bottomLeft >= LAVA_TILE_START && bottomLeft <= LAVA_TILE_END) ||
		(topRight >= LAVA_TILE_START && topRight <= LAVA_TILE_END) ||
		(bottomRight >= LAVA_TILE_START && bottomRight <= LAVA_TILE_END))
	{
		e->environment = LAVA;
	}

	else
	{
		y2 = (e->y + (e->h / 2)) / TILE_SIZE;

		bottomLeft  = mapTileAt(x1, y2);
		bottomRight = mapTileAt(x2, y2);

		if ((topLeft >= WATER_TILE_START && topLeft <= WATER_TILE_END) &&
			(bottomLeft >= WATER_TILE_START && bottomLeft <= WATER_TILE_END) &&
			(topRight >= WATER_TILE_START && topRight <= WATER_TILE_END) &&
			(bottomRight >= WATER_TILE_START && bottomRight <= WATER_TILE_END))
		{
			e->environment = WATER;

		}
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
			if (collision(e->x + (e->face == LEFT ? 0 : e->w), e->y + e->h, 1, 1, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == TRUE)
			{
				return FALSE;
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
