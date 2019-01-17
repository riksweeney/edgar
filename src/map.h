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

void loadMap(char *, int);
int saveMap(void);
void doMap(void);
void drawMap(int);
void centerEntityOnMap(void);
void freeMap(void);
Texture *tileImage(int);
Texture *mapImageAt(int, int);
int mapTileAt(int, int);
int getMapMaxX(void);
int getMapMaxY(void);
void setMapMaxX(int);
void setMapMaxY(int);
void setMapMinX(int);
void setMapMinY(int);
int getMapMinX(void);
int getMapMinY(void);
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
int isDarkMap(void);
void setDarkMap(int);
void resetBlendTime(void);
int getCameraMinX(void);
int getCameraMinY(void);
int getCameraMaxX(void);
int getCameraMaxY(void);
int getPlayerMinX(void);
int getPlayerMaxX(void);
void drawMapBackground(void);
void limitPlayerToCameraLimits(void);
void setCameraPositionFromScript(char *);
