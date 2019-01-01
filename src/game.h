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

void initGame(void);
void doGame(void);
void drawGame(void);
void shakeScreen(int, int);
void setTransition(int, void (*)(void));
void setNextLevel(char *, char *);
void goToNextMap(void);
void setCheckpoint(float, float);
void getCheckpoint(float *, float *);
void pauseGame(void);
void showPauseDialog(void);
void freeGame(void);
void setNextLevelFromScript(char *);
void writeGameSettingsToFile(FILE *);
void readGameSettingsFromFile(char *);
void resetGameSettings(void);
void initGameOver(void);
void doGameOver(void);
void drawGameOver(void);
void focusLost(void);
void showSaveDialog(void);
void pauseGameInventory(void);
char *getPlayTimeAsString(void);
void fadeFromColour(int, int, int, int);
void increaseKillCount(void);
char *getSlimeTimeAsString(void);
void increaseSecretsFound(void);
void setContinuePoint(int, char *, void (*)(void));
void getContinuePoint(void);
void clearContinuePoint(void);
char *getPlayTimeHours(void);
void drawWeather(void);
void updateExitCount(int);
void showEndCredits(void);
