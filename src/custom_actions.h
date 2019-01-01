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

void setCustomAction(Entity *, void (*)(int *, int *, float *), int, int, float);
void doCustomAction(CustomAction *);
int hasCustomAction(Entity *, void (*func)(int *, int *, float *));
void helpless(int *, int *, float *);
void invulnerable(int *, int *, float *);
void invulnerableNoFlash(int *, int *, float *);
void flashWhite(int *, int *, float *);
void slowDown(int *, int *, float *);
void clearCustomActions(Entity *);
void clearCustomAction(Entity *, void (*)(int *, int *, float *));
void dizzy(int *, int *, float *);
void regenerate(int *, int *, float *);
void addCustomActionFromScript(Entity *, char *);
void slimeTimeout(int *, int *, float *);
void invisible(int *, int *, float *);
void antiGravity(int *, int *, float *);
void attract(int *, int *, float *);
void spriteTrail(int *, int *, float *);
void stickToFloor(int *, int *, float *);
void removeFriction(int *, int *, float *);
void confused(int *, int *, float *);
