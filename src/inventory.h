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

int addToInventory(Entity *);
void moveInventoryCursor(int);
void selectInventoryItem(void);
Entity *getInventoryItemByObjectiveName(char *);
void useInventoryItem(void);
void doInventory(void);
void drawSelectedInventoryItem(int, int, int, int);
int removeInventoryItemByObjectiveName(char *);
void addRequiredToInventory(Entity *);
void loadInventoryItems(void);
void writeInventoryToFile(FILE *);
void getInventoryItemFromScript(char *);
void freeInventory(void);
void drawInventory(void);
void nextInventoryItem(int);
void setInventoryDialogMessage(char *, ...);
void clearInventoryDescription(void);
void useInventoryItemFromScript(char *);
void replaceInventoryItem(char *, Entity *);
void replaceInventoryItemWithName(char *, Entity *);
Entity *getCurrentInventoryItem(void);
void setInventoryIndex(int);
int getInventoryIndex(void);
int hasBow(void);
int hasLightningSword(void);
void scriptAddToInventory(char *, int);
void moveInventoryItem(int);
Entity *removeInventoryItemAtCursor(void);
void resetInventoryIndex(void);
Entity *getInventoryItemByName(char *);
int removeInventoryItemByName(char *);
