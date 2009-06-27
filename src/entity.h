/*
Copyright (C) 2009 Parallel Realities

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

void standardDie(void);
void freeEntities(void);
Entity *getFreeEntity(void);
void doEntities(void);
void drawEntities(int);
void removeEntity(void);
void doNothing(void);
void entityDie(void);
void standardDie(void);
void entityTakeDamageNoFlinch(Entity *, int);
void entityTakeDamageFlinch(Entity *, int);
void entityTouch(Entity *);
void pushEntity(Entity *);
int addEntity(Entity, int, int);
Entity *getEntityByObjectiveName(char *);
void activateEntitiesWithRequiredName(char *, int);
void activateEntitiesWithObjectiveName(char *, int);
void interactWithEntity(int, int, int, int);
void initLineDefs(void);
void writeEntitiesToFile(FILE *);
void floatLeftToRight(void);
void moveLeftToRight(void);
void writeEntityToFile(Entity *, FILE *);
void flyLeftToRight(void);
void flyToTarget(void);
void addEntityFromScript(char *);
void entityWalkTo(Entity *, char *);
void entityWalkToRelative(Entity *, char *);
void changeDirection(void);
void changeTarget(void);
void entityDieNoDrop(void);
void noItemDie(void);
void rotateAroundStartPoint(void);
void enemyPain(void);
Entity *getEntityByStartXY(int, int);
