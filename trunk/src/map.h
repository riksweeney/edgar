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

void loadMap(char *, int);
int saveMap(void);
void doMap(void);
void drawMap(int);
void centerEntityOnMap(void);
void freeMap(void);
SDL_Surface *tileImage(int);
SDL_Surface *mapImageAt(int, int);
int mapTileAt(int, int);
int getMaxMapX(void);
int getMaxMapY(void);
void setMaxMapX(int);
void setMaxMapY(int);
void setMinMapX(int);
void setMinMapY(int);
int getMinMapX(void);
int getMinMapY(void);
int mapStartX(void);
int mapStartY(void);
void mapStartXNext(int);
void mapStartYNext(int);
void setTileAt(int, int, int);
int nextTile(int);
int prevTile(int);
void centerMapOnEntity(Entity *);
int getMapStartX(void);
int getMapStartY(void);
void setMapStartX(int);
void setMapStartY(int);
char *getMapFilename(void);
void cameraSnapToTargetEntity(void);
void setCameraPosition(int, int);
void limitCamera(int, int, int, int);
void resetCameraLimits(void);
void limitCameraFromScript(char *);
char *getMapMusic(void);
void setCameraSpeed(float);
int cameraAtMinimum(void);
char *getMapName(void);
int getDistanceFromCamera(int, int);
int outOfBounds(Entity *);
int mapIsDark(void);
