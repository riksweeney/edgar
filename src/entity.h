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

void standardDie(void);
void freeEntities(void);
Entity *getFreeEntity(void);
void doEntities(void);
void drawEntities(int);
void removeEntity(void);
void removeAllSpawnedIn(void);
void disableSpawners(int);
void doNothing(void);
void entityDie(void);
void standardDie(void);
void entityTakeDamageNoFlinch(Entity *, int);
void entityTakeDamageFlinch(Entity *, int);
void entityTouch(Entity *);
void pushEntity(Entity *);
Entity *addEntity(Entity, int, int);
Entity *getEntityByObjectiveName(char *);
Entity *getEntityByRequiredName(char *);
void activateEntitiesWithRequiredName(char *, int);
void activateEntitiesWithObjectiveName(char *, int);
void interactWithEntity(int, int, int, int);
void initLineDefs(void);
void writeEntitiesToFile(FILE *);
void floatLeftToRight(void);
void moveLeftToRight(void);
void flyLeftToRight(void);
void syncBoulderFrameSpeed(void);
void addEntityFromScript(char *);
void entityWalkTo(Entity *, char *);
void entityWalkToRelative(Entity *, char *);
void changeDirection(Entity *);
void entityDieNoDrop(void);
void noItemDie(void);
void rotateAroundStartPoint(void);
void enemyPain(void);
Entity *getEntityByStartXY(int, int);
int countSiblings(Entity *, int *);
void doTeleport(void);
void activateEntitiesValueWithObjectiveName(char *, int);
int getLeftEdge(Entity *);
int getRightEdge(Entity *);
void entityDieVanish(void);
void killEntity(char *);
void freeEntityList(EntityList *);
void addEntityToList(EntityList *, Entity *);
EntityList *getEntitiesByObjectiveName(char *);
EntityList *getEntitiesByRequiredName(char *);
int atTarget(void);
void faceTarget(void);
void addToDrawLayer(Entity *, int);
void clearDrawLayers(void);
Entity *getEntityByName(char *);
void teleportEntityFromScript(Entity *, char *);
int landedOnGround(long);
EntityList *getEntitiesByName(char *);
void addDuplicateImage(Entity *);
void entityWalkToEntity(Entity *, char *);
EntityList *getEntities(void);
Entity *getEntityByXY(int, int);
