/*
Copyright (C) 2009-2012 Parallel Realities

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

void loadAnimationData(char *, int *, EntityAnimation *);
void freeAnimations(void);
void drawLoopingAnimation(Entity *, int, int, int, int, int);
int drawLoopingAnimationToMap(void);
#if DEV == 1
void setEntityAnimation(Entity *, char *) __attribute__((nonnull(2)));
#else
void setEntityAnimation(Entity *, char *);
#endif
void setEntityAnimationByID(Entity *, int);
int drawLineDefToMap(void);
int drawPhaseDoorToMap(void);
char *getAnimationTypeAtIndex(Entity *);
int hasEntityAnimation(Entity *, char *);
void setFrameData(Entity *);
int drawSpriteToMap(void);
Sprite *getCurrentSprite(Entity *);
int getFrameCount(Entity *);
void drawSprite(Entity *, int, int, int, int, int);
