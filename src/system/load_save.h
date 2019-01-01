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

void saveGame(int);
int loadGame(int);
void saveTemporaryData(void);
void setupUserHomeDirectory(void);
void loadPersitanceData(char *);
int hasPersistance(char *);
void saveConfig(void);
void loadConfig(void);
void newGame(void);
char *getGameSavePath(void);
char **getSaveFileIndex(void);
void tutorial(void);
int loadContinueData(void);
void saveContinueData(void);
int bossExists(char *);
void startOnMap(char *);
void loadObtainedMedals(void);
void saveObtainedMedals(void);
void titleScreen(void);
int getMostRecentSave(void);
