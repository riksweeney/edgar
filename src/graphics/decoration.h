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

void freeDecorations(void);
Entity *getFreeDecoration(void);
void doDecorations(void);
void drawDecorations(void);
void addStarExplosion(int, int);
void addSparkle(int, int);
Entity *addTrail(int, int, char *, int);
Entity *addDecoration(char *, int, int);
Entity *addSmoke(int, int, char *);
Entity *addMultipleSparkles(int, int, char *);
void addDecorationFromScript(char *);
void addBlood(int, int);
Entity *addBasicDecoration(int, int, char *);
void addParticleExplosion(int, int);
Entity *addPixelDecoration(int, int);
Entity *addParticle(int, int);
