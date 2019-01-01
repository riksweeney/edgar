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

#include "headers.h"

#include "audio/audio.h"
#include "collisions.h"
#include "entity.h"
#include "map.h"

extern Entity *self;
extern Entity player, playerShield, playerWeapon;

static Grid grid[GRID_MAX_Y][GRID_MAX_X];

static void addToList(int, int, Entity *);

void initCollisionGrid()
{
	int x, y;

	for (y=0;y<GRID_MAX_Y;y++)
	{
		for (x=0;x<GRID_MAX_X;x++)
		{
			grid[y][x].count = 0;
			grid[y][x].listHead.next = NULL;
		}
	}
}

void addToGrid(Entity *e)
{
	int left, right, top, bottom, x, y;

	if (e == &playerWeapon)
	{
		if (e->face == LEFT)
		{
			left = (e->x - e->w + e->offsetX - 1) / TILE_SIZE / GRID_SIZE;
			right = (e->x + e->offsetX - 1) / TILE_SIZE / GRID_SIZE;
		}

		else
		{
			left = (e->x + e->offsetX) / TILE_SIZE / GRID_SIZE;
			right = (e->x + e->offsetX + e->w) / TILE_SIZE / GRID_SIZE;
		}

		top = e->y / TILE_SIZE / GRID_SIZE;
		bottom = (e->y + e->h) / TILE_SIZE / GRID_SIZE;
	}

	else
	{
		left = getLeftEdge(e) / TILE_SIZE / GRID_SIZE;
		right = getRightEdge(e) / TILE_SIZE / GRID_SIZE;

		top = (e->y + e->box.y) / TILE_SIZE / GRID_SIZE;
		bottom = (e->y + e->box.y + e->box.h) / TILE_SIZE / GRID_SIZE;
	}

	for (x=left;x<=right;x++)
	{
		for (y=top;y<=bottom;y++)
		{
			addToList(y, x, e);
		}
	}
}

static void addToList(int y, int x, Entity *e)
{
	EntityList *listHead, *list;

	if (x < 0 || x >= GRID_MAX_X || y < 0 || y >= GRID_MAX_Y)
	{
		return;
	}

	listHead = &grid[y][x].listHead;

	while (listHead->next != NULL)
	{
		listHead = listHead->next;
	}

	list = malloc(sizeof(EntityList));

	list->entity = e;
	list->next = NULL;

	listHead->next = list;

	grid[y][x].count++;
}

void freeCollisionGrid()
{
	int x, y;
	EntityList *p, *q;

	for (y=0;y<GRID_MAX_Y;y++)
	{
		for (x=0;x<GRID_MAX_X;x++)
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
	int i, j, x1, y1, x2, y2, w1, h1, w2, h2;
	Entity *e1, *e2, *temp;
	EntityList *list1, *list2;

	for (i=0;i<GRID_MAX_Y;i++)
	{
		for (j=0;j<GRID_MAX_X;j++)
		{
			for (list1=grid[i][j].listHead.next;list1!=NULL;list1=list1->next)
			{
				e1 = list1->entity;

				if (e1->flags & TELEPORTING)
				{
					continue;
				}

				if (e1->inUse == TRUE && e1->touch != NULL)
				{
					for (list2=grid[i][j].listHead.next;list2!=NULL;list2=list2->next)
					{
						e2 = list2->entity;

						if ((e1->type != PLAYER && (e2->flags & PLAYER_TOUCH_ONLY)) ||
							((e1->flags & PLAYER_TOUCH_ONLY) && e2->type != PLAYER))
						{
							continue;
						}

						if ((e1->type == CONVEYOR_BELT && e2->type == CONVEYOR_BELT) ||
							(e1->type == WEAK_WALL && e2->type == WEAK_WALL) ||
							(e1->type == ANTI_GRAVITY && e2->type == ANTI_GRAVITY))
						{
							continue;
						}

						if (e2->flags & TELEPORTING)
						{
							continue;
						}

						if (e1 != e2 && e2->inUse == TRUE && e2->touch != NULL)
						{
							if (e1->type == PROJECTILE)
							{
								if (e2->type == PROJECTILE || (e1->parent != NULL && (e1->parent->type == ENEMY || e1->parent->type == SPAWNER) && e2->type == ENEMY))
								{
									continue;
								}
							}

							if ((e1 == &player && e2 == &playerWeapon) || (e1 == &playerWeapon && e2 == &player))
							{
								continue;
							}

							x1 = getLeftEdge(e1);

							y1 = e1->y + e1->box.y;

							w1 = e1->box.w;
							h1 = e1->box.h;

							x2 = getLeftEdge(e2);

							y2 = e2->y + e2->box.y;

							w2 = e2->box.w;
							h2 = e2->box.h;

							if (e1 == &playerWeapon)
							{
								if (e1->face == LEFT)
								{
									x1 = e1->parent->x - e1->w + e1->offsetX - 1;
								}

								else
								{
									x1 = e1->x + e1->offsetX;
								}


								y1 = e1->parent->y + e1->offsetY;
							}

							else if (e2 == &playerWeapon)
							{
								if (e2->face == LEFT)
								{
									x2 = e2->parent->x - e2->w + e2->offsetX - 1;
								}

								else
								{
									x2 = e2->x + e2->offsetX;
								}


								y2 = e2->parent->y + e2->offsetY;
							}

							if (collision(x1, y1, w1, h1, x2, y2, w2, h2) == TRUE)
							{
								temp = self;

								self = e2;

								self->touch(e1);

								self = temp;
							}
						}

						if (e1->inUse == FALSE)
						{
							break;
						}
					}
				}
			}
		}
	}
}

Entity *checkEntityToEntity(Entity *e)
{
	int i, j, x1, y1, x2, y2, w1, h1, w2, h2;
	Entity *e1, *e2;
	EntityList *list1, *list2;

	for (i=0;i<GRID_MAX_Y;i++)
	{
		for (j=0;j<GRID_MAX_X;j++)
		{
			for (list1=grid[i][j].listHead.next;list1!=NULL;list1=list1->next)
			{
				e1 = list1->entity;

				if (e1 != e)
				{
					continue;
				}

				if (e1->inUse == TRUE)
				{
					for (list2=grid[i][j].listHead.next;list2!=NULL;list2=list2->next)
					{
						e2 = list2->entity;

						if (e1 != e2 && e2->inUse == TRUE && e2->touch != NULL && e2->weight > 0)
						{
							if (e1->type == ENEMY && e2->type == ENEMY)
							{
								continue;
							}

							if (e1->type == ANTI_GRAVITY || e2->type == ANTI_GRAVITY)
							{
								continue;
							}

							if ((e1->type != PLAYER && (e2->flags & PLAYER_TOUCH_ONLY)) ||
								((e1->flags & PLAYER_TOUCH_ONLY) && e2->type != PLAYER))
							{
								continue;
							}

							if (e1->type == PROJECTILE)
							{
								if (e2->type == PROJECTILE || (e1->parent != NULL && e1->parent->type == ENEMY && e2->type == ENEMY))
								{
									continue;
								}
							}

							if ((e1 == &player && e2 == &playerWeapon) || (e1 == &playerWeapon && e2 == &player))
							{
								continue;
							}

							x1 = getLeftEdge(e1);

							y1 = e1->y + e1->box.y;

							w1 = e1->box.w;
							h1 = e1->box.h;

							x2 = getLeftEdge(e2);

							y2 = e2->y + e2->box.y;

							w2 = e2->box.w;
							h2 = e2->box.h;

							if (e1 == &playerWeapon)
							{
								x1 = e1->x + e1->box.x;
								y1 = e1->y + e1->box.y;

								if (e1->face == LEFT)
								{
									x1 += e1->parent->w - e1->w - e1->offsetX;
								}

								else
								{
									x1 += e1->offsetX;
								}


								y1 += e1->offsetY;
							}

							if (collision(x1, y1, w1, h1, x2, y2, w2, h2) == TRUE)
							{
								return e2;
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}

Entity *isSpaceEmpty(Entity *e)
{
	EntityList *el, *entities;

	entities = getEntities();

	if (player.inUse == TRUE && collision(e->x, e->y, e->w, e->h, player.x, player.y, player.w, player.h) == 1)
	{
		return &player;
	}

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && e != el->entity
			&& collision(e->x, e->y, e->w, e->h, el->entity->x, el->entity->y, el->entity->w, el->entity->h) == 1)
		{
			return el->entity;
		}
	}

	return NULL;
}

int isNearObstacle(Entity *e)
{
	EntityList *el, *entities;

	entities = getEntities();

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && (el->entity->flags & (OBSTACLE|PUSHABLE))
			&& collision(e->x, e->y, e->w, e->h, el->entity->x, el->entity->y, el->entity->w, el->entity->h) == 1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void checkToMap(Entity *e)
{
	int i, x1, x2, y1, y2, previousEnvironment;
	int topLeft, topRight, bottomLeft, bottomRight, previousY2, previous;
	int tempX, tempY, wasOnGround;

	wasOnGround = (e->flags & ON_GROUND);

	/* Remove the entity from the ground */

	e->flags &= ~ON_GROUND;

	/* Set environment to air */

	previousEnvironment = e->environment;

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

				previous = (e->x + e->w - 1) / TILE_SIZE;

				previous = mapTileAt(previous, y2);

				if (previous >= SLOPE_UP_START && previous <= SLOPE_UP_END)
				{
					if (i == e->h)
					{
						if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
						{
							e->y -= e->dirX;

							e->dirY = 0;

							e->flags |= ON_GROUND;
						}

						else if (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START)
						{
							e->y = (int)((e->y + e->h - 1) / TILE_SIZE);

							e->y *= TILE_SIZE;

							e->y -= e->h + 0.5;

							e->dirY = 0;

							e->flags |= ON_GROUND;

							previous = mapTileAt(x2, (e->y + e->h - 1) / TILE_SIZE);

							if (previous >= SLOPE_UP_START && previous <= SLOPE_UP_END)
							{
								e->y -= e->dirX;

								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}
				}

				else if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					if (i == e->h)
					{
						if (!(e->flags & FLY))
						{
							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}

						else
						{
							e->x = x2 * TILE_SIZE;

							e->x -= e->w;

							e->dirX = 0;
						}

						if (e->type == PROJECTILE)
						{
							e->die();

							return;
						}
					}
				}

				else if ((previous >= SLOPE_UP_START && previous <= SLOPE_UP_END) &&
					!(bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END))
				{
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					e->flags |= ON_GROUND;
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

					e->dirX = (e->flags & BOUNCES) ? -e->dirX : 0;

					if ((e->flags & GRABBING) && e->target != NULL)
					{
						e->target->dirX = 0;
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}
			}

			else if (e->dirX < 0)
			{
				/* Trying to move left */

				previous = (e->x) / TILE_SIZE;

				previous = mapTileAt(previous, y2);

				if (previous >= SLOPE_DOWN_START && previous <= SLOPE_DOWN_END)
				{
					if (i == e->h)
					{
						if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
						{
							e->y += e->dirX;

							e->dirY = 0;

							e->flags |= ON_GROUND;
						}

						else if (bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START)
						{
							e->y = (int)((e->y + e->h - 1) / TILE_SIZE);

							e->y *= TILE_SIZE;

							e->y -= e->h + 0.5;

							e->dirY = 0;

							e->flags |= ON_GROUND;

							previous = mapTileAt(x1, (e->y + e->h - 1) / TILE_SIZE);

							if (previous >= SLOPE_DOWN_START && previous <= SLOPE_DOWN_END)
							{
								e->y += e->dirX;

								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					if (i == e->h)
					{
						if (!(e->flags & FLY))
						{
							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}

						else
						{
							e->x = (x1 + 1) * TILE_SIZE;

							e->dirX = 0;
						}

						if (e->type == PROJECTILE)
						{
							e->die();

							return;
						}
					}
				}

				else if ((previous >= SLOPE_DOWN_START && previous <= SLOPE_DOWN_END) &&
					!(bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END))
				{
				}

				else if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					e->flags |= ON_GROUND;
				}

				else if ((topLeft >= JUMP_THROUGH_TILE_START && topLeft <= JUMP_THROUGH_TILE_END) ||
					((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topLeft != BLANK_TILE && topLeft < BACKGROUND_TILE_START) || (bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->x = (x1 + 1) * TILE_SIZE;

					e->dirX = (e->flags & BOUNCES) ? -e->dirX : 0;

					if ((e->flags & GRABBING) && e->target != NULL)
					{
						e->target->dirX = 0;
					}

					if (e->type == PROJECTILE)
					{
						e->die();

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
		y2 = (e->y + e->dirY + e->h - 1) / TILE_SIZE;

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
						tempX = (int)(e->x + i - 1) % TILE_SIZE;
						tempY = (int)(e->y + e->dirY + e->h - 1) % TILE_SIZE;

						tempX = TILE_SIZE - tempX;

						if (tempY > tempX || wasOnGround != 0)
						{
							e->y = y2 * TILE_SIZE;
							e->y -= e->h;
							e->y += tempX + 1;

							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					if (i == (e->w > TILE_SIZE ? TILE_SIZE : e->w))
					{
						tempX = (int)(e->x) % TILE_SIZE;
						tempY = (int)(e->y + e->dirY + e->h - 1) % TILE_SIZE;

						if (tempY > tempX || wasOnGround != 0)
						{
							e->y = y2 * TILE_SIZE;
							e->y -= e->h;
							e->y += tempX + 1;

							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}

				else if ((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END) ||
					((bottomRight >= JUMP_THROUGH_TILE_START && bottomRight <= JUMP_THROUGH_TILE_END)))
				{
					previousY2 = y2;

					x1 = (e->x) / TILE_SIZE;
					x2 = (e->x + i - 1) / TILE_SIZE;

					y2 = (e->y + e->h - 1) / TILE_SIZE;

					bottomLeft  = mapTileAt(x1, y2);
					bottomRight = mapTileAt(x2, y2);

					if (!((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END) ||
						((bottomRight >= JUMP_THROUGH_TILE_START && bottomRight <= JUMP_THROUGH_TILE_END))))
					{
						y2 = previousY2;

						/* Place the player as close to the solid tile as possible */

						e->y = y2 * TILE_SIZE;
						e->y -= e->h;

						if ((e->flags & BOUNCES) && e->dirY > 4)
						{
							e->dirY = -e->dirY * 2 / 3;
						}

						else
						{
							e->dirY = 0;

							e->flags |= ON_GROUND;
						}
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}

				else if ((bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START) || (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->y = y2 * TILE_SIZE;
					e->y -= e->h;

					if ((e->flags & BOUNCES) && e->dirY > 4)
					{
						e->dirY = -e->dirY * 2 / 3;
					}

					else
					{
						e->dirY = 0;

						e->flags |= ON_GROUND;
					}

					if (e->type == PROJECTILE)
					{
						e->die();

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

					e->dirY = (e->flags & BOUNCES) ? -e->dirY : 0;

					if (e->type == PROJECTILE)
					{
						e->die();

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

	x1 = (e->type == PLAYER || (e->flags & LIMIT_TO_SCREEN)) ? getPlayerMinX() : getMapMinX();
	x2 = (e->type == PLAYER || (e->flags & LIMIT_TO_SCREEN)) ? getPlayerMaxX() : getMapMaxX();

	y1 = (e->flags & LIMIT_TO_SCREEN) ? getMapStartY() : getMapMinY() - 300;

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
			e->die();
		}

	}

	else if (e->x + e->w >= x2)
	{
		e->x = x2 - e->w - 1;

		e->dirX = 0;

		if ((e->flags & GRABBING) && e->target != NULL)
		{
			e->target->dirX = 0;
		}

		if (e->type == PROJECTILE)
		{
			e->die();
		}
	}

	if (e->y > getMapMaxY() && e->y - e->dirY <= getMapMaxY())
	{
		e->flags &= ~(HELPLESS|INVULNERABLE);

		if (e->fallout == NULL)
		{
			printf("%s has no fallout defined. Removing\n", e->name);

			e->inUse = FALSE;

			return;
		}

		else
		{
			e->fallout();
		}
	}

	else if (e->y < y1)
	{
		/* Way too high... */

		e->y = y1;

		e->dirY = 0;
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

		if (previousEnvironment != LAVA && e->fallout != NULL)
		{
			/* Fire based entities won't die */

			if (e->element != FIRE)
			{
				if (e->type != TEMP_ITEM)
				{
					playSoundToMap("sound/common/lava", -1, e->x, e->y, 0);
				}

				e->flags &= ~(HELPLESS|INVULNERABLE);

				e->fallout();
			}
		}
	}

	else if ((topLeft >= SLIME_TILE_START && topLeft <= SLIME_TILE_BLEND) ||
		(bottomLeft >= SLIME_TILE_START && bottomLeft <= SLIME_TILE_BLEND) ||
		(topRight >= SLIME_TILE_START && topRight <= SLIME_TILE_BLEND) ||
		(bottomRight >= SLIME_TILE_START && bottomRight <= SLIME_TILE_BLEND))
	{
		e->environment = SLIME;

		if (previousEnvironment != SLIME && e->fallout != NULL)
		{
			/* Slime based entities won't die */

			if (e->element != SLIME)
			{
				if (e->type != TEMP_ITEM)
				{
					playSoundToMap("sound/common/slime", -1, e->x, e->y, 0);
				}

				e->flags &= ~(HELPLESS|INVULNERABLE);

				e->fallout();
			}
		}
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

			if (previousEnvironment != WATER && e->fallout != NULL)
			{
				if (e->type != TEMP_ITEM)
				{
					playSoundToMap("sound/common/splash", -1, e->x, e->y, 0);
				}

				if (!(e->flags & FLOATS))
				{
					e->flags &= ~(HELPLESS|INVULNERABLE);

					e->fallout();
				}
			}
		}
	}
}

int isAtEdge(Entity *e)
{
	int i, tile;
	int x = e->face == LEFT ? floor(e->x) : ceil(e->x) + e->w;
	int y = e->y + e->h - 1;
	EntityList *el, *entities;

	entities = getEntities();

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	tile = mapTileAt(x, y);

	/* Return immediately if the tile isn't blank */

	if (!(e->flags & ON_GROUND) || (tile != BLANK_TILE && tile < BACKGROUND_TILE_START))
	{
		return FALSE;
	}

	if (e->w > TILE_SIZE)
	{
		if (e->dirX > 0)
		{
			for (i=0;;)
			{
				x = e->x + i;

				x /= TILE_SIZE;

				tile = mapTileAt(x, y);

				if (tile >= SLOPE_DOWN_START && tile <= SLOPE_DOWN_END)
				{
					return FALSE;
				}

				if (i == e->w)
				{
					break;
				}

				i += TILE_SIZE;

				if (i > e->w)
				{
					i = e->w;
				}
			}
		}

		else
		{
			for (i=e->w;;)
			{
				x = e->x + i;

				x /= TILE_SIZE;

				tile = mapTileAt(x, y);

				if (tile >= SLOPE_UP_START && tile <= SLOPE_UP_END)
				{
					return FALSE;
				}

				if (i == 0)
				{
					break;
				}

				i -= TILE_SIZE;

				if (i < 0)
				{
					i = 0;
				}
			}
		}
	}

	x = e->face == LEFT ? floor(e->x) : ceil(e->x);

	if (e->face == RIGHT)
	{
		x += e->w;
	}

	/* There might still be Entities that can be walked on */

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (e != el->entity && el->entity->inUse == TRUE && el->entity->touch != NULL
			&& ((el->entity->flags & (PUSHABLE|OBSTACLE)) || (el->entity->type == WEAK_WALL)
			|| (el->entity->type == PRESSURE_PLATE) || (el->entity->type == ANTI_GRAVITY)))
		{
			if (collision(x, e->y, 1, e->h + 10, el->entity->x, el->entity->y, el->entity->w, el->entity->h) == TRUE)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

int isAtCeilingEdge(Entity *e)
{
	int i, tile;
	int x = e->x + (e->dirX > 0 ? e->w : 0);
	int y = e->y;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y--;

	tile = mapTileAt(x, y);

	/* Return immediately if the tile isn't blank */

	if (tile != BLANK_TILE && tile < BACKGROUND_TILE_START)
	{
		return FALSE;
	}

	if (e->w > TILE_SIZE)
	{
		if (e->dirX > 0)
		{
			for (i=0;;)
			{
				x = e->x + i;

				x /= TILE_SIZE;

				tile = mapTileAt(x, y);

				if (tile >= SLOPE_DOWN_START && tile <= SLOPE_DOWN_END)
				{
					return FALSE;
				}

				if (i == e->w)
				{
					break;
				}

				i += TILE_SIZE;

				if (i > e->w)
				{
					i = e->w;
				}
			}
		}

		else
		{
			for (i=e->w;;)
			{
				x = e->x + i;

				x /= TILE_SIZE;

				tile = mapTileAt(x, y);

				if (tile >= SLOPE_UP_START && tile <= SLOPE_UP_END)
				{
					return FALSE;
				}

				if (i == 0)
				{
					break;
				}

				i -= TILE_SIZE;

				if (i < 0)
				{
					i = 0;
				}
			}
		}
	}

	return TRUE;
}

int onSingleTile(Entity *e)
{
	int leftTile, rightTile, midTile, wallLeft, wallRight;
	int x = e->x + e->w / 2;
	int y = e->y + e->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	midTile = mapTileAt(x, y);

	leftTile = mapTileAt(x - 1, y);

	rightTile = mapTileAt(x + 1, y);

	wallLeft = mapTileAt(x - 1, y - 1);

	wallRight = mapTileAt(x + 1, y - 1);

	/* On a tile with nothing either side */
	if ((midTile != BLANK_TILE && midTile < BACKGROUND_TILE_START) && leftTile == BLANK_TILE && rightTile == BLANK_TILE)
	{
		return TRUE;
	}

	/* On a tile with nothing on the left and a wall on the right */
	if ((midTile != BLANK_TILE && midTile < BACKGROUND_TILE_START) && leftTile == BLANK_TILE && (wallRight != BLANK_TILE && wallRight < BACKGROUND_TILE_START))
	{
		return TRUE;
	}

	/* On a tile with nothing on the right and a wall on the left */
	if ((midTile != BLANK_TILE && midTile < BACKGROUND_TILE_START) && rightTile == BLANK_TILE && (wallLeft != BLANK_TILE && wallLeft < BACKGROUND_TILE_START))
	{
		return TRUE;
	}

	return FALSE;
}

int isValidOnMap(Entity *e)
{
	int i, x1, x2, y1, y2;

	if (e->x < getMapMinX() || e->x + e->w > getMapMaxX() || e->h < 0 || e->y + e->h > getMapMaxY())
	{
		return FALSE;
	}

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

int getMapFloor(int x, int y)
{
	int maxY = getMapMaxY() / TILE_SIZE;
	int tileID;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	tileID = mapTileAt(x, y);

	while (tileID == BLANK_TILE || (tileID >= BACKGROUND_TILE_START && tileID <= BACKGROUND_TILE_END))
	{
		y++;

		if (y >= maxY)
		{
			break;
		}

		tileID = mapTileAt(x, y);
	}

	y *= TILE_SIZE;

	return y;
}

int getMapCeiling(int x, int y)
{
	int tileID;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	tileID = mapTileAt(x, y);

	while (tileID == BLANK_TILE || (tileID >= BACKGROUND_TILE_START && tileID <= BACKGROUND_TILE_END))
	{
		y--;

		if (y < 0)
		{
			break;
		}

		tileID = mapTileAt(x, y);
	}

	y = y * TILE_SIZE + TILE_SIZE;

	return y;
}

int getMapLeft(int x, int y)
{
	int tileID;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	tileID = mapTileAt(x, y);

	while (tileID == BLANK_TILE || (tileID >= BACKGROUND_TILE_START && tileID <= BACKGROUND_TILE_END))
	{
		x--;

		if (x <= 0)
		{
			break;
		}

		tileID = mapTileAt(x, y);
	}

	x *= TILE_SIZE;

	return x;
}

int getMapRight(int x, int y)
{
	int maxX = getMapMaxX() / TILE_SIZE;
	int tileID;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	tileID = mapTileAt(x, y);

	while (tileID == BLANK_TILE || (tileID >= BACKGROUND_TILE_START && tileID <= BACKGROUND_TILE_END))
	{
		x++;

		if (x >= maxX)
		{
			break;
		}

		tileID = mapTileAt(x, y);
	}

	x *= TILE_SIZE;

	return x;
}

int getWaterTop(int x, int y)
{
	int tileID;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	tileID = mapTileAt(x, y);

	while (tileID == WATER_TILE_START)
	{
		y--;

		if (y < 0)
		{
			break;
		}

		tileID = mapTileAt(x, y);
	}

	y++;

	y *= TILE_SIZE;

	return y;
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
