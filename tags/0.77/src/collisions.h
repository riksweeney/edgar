/*
Copyright (C) 2009-2011 Parallel Realities

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

int collision(int, int, int, int, int, int, int, int);
void doCollisions(void);
void checkToMap(Entity *);
int isAtEdge(Entity *);
int isAtCeilingEdge(Entity *);
int isValidOnMap(Entity *);
Entity *isSpaceEmpty(Entity *);
Entity *checkEntityToEntity(Entity *);
void addToGrid(Entity *);
void initCollisionGrid(void);
void freeCollisionGrid(void);
int getMapFloor(int, int);
int getMapCeiling(int, int);
int getMapLeft(int, int);
int getMapRight(int, int);
int isNearObstacle(Entity *);
int getWaterTop(int, int);
