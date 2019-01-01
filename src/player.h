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
EntityList *playerGib(void);
void facePlayer(void);
void setPlayerSlimed(int);
void setBowAmmo(int);
void setPlayerStunned(int);
void doStunned(void);
void becomeJumpingSlime(int);
void becomeEdgar(void);
void setPlayerLocked(int);
Entity *removePlayerWeapon(void);
Entity *removePlayerShield(void);
void setPlayerFrozen(int);
void setPlayerWrapped(int);
int isAttacking(void);
void playerStand(void);
void addChargesToWeapon(void);
int isPlayerLocked(void);
void playerWaitForConfirm(void);
void setWeaponFromScript(char *);
void scriptAttack(void);
void setShieldFromScript(char *);
void unsetWeapon(void);
void unsetShield(void);
void setPlayerConfused(int);
void setPlayerPetrified(void);
void setPlayerAsh(void);
