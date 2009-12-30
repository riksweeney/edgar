/*
Copyright (C) 2009-2010 Parallel Realities

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

void setCustomAction(Entity *, void (*)(int *, int *, float *), int, int, float);
void doCustomAction(CustomAction *);
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
