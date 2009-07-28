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

Entity *loadPlayer(int, int, char *);
void setPlayerLocation(int, int);
void doPlayer(void);
void drawPlayer(void);
void setPlayerShield(int);
void setPlayerWeapon(int);
void autoSetPlayerWeapon(Entity *);
void autoSetPlayerShield(Entity *);
int getDistanceFromPlayer(Entity *);
void writePlayerToFile(FILE *);
void setPlayerWeaponName(char *);
void setPlayerShieldName(char *);
void playerWaitForDialog(void);
void playerResumeNormal(void);
void writePlayerMapStartToFile(FILE *);
void increasePlayerMaxHealth(void);
void syncWeaponShieldToPlayer(void);
void freePlayer(void);
void resetPlayerAnimations(void);
void playerGib(void);
void facePlayer(void);
void setPlayerSlimed(int);
void setBowAmmo(int);
void setPlayerStunned(void);
