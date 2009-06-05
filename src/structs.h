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

#include "defs.h"

typedef struct Label
{
	int x, y;
	SDL_Surface *text;
} Label;

typedef struct Widget
{
	int x, y;
	int *value, maxValue, minValue;
	void (*clickAction)(void);
	void (*rightAction)(void);
	void (*leftAction)(void);
	Label *label;
	SDL_Surface *normalState, *selectedState;
} Widget;

typedef struct Menu
{
	int index, x, y, w, h, widgetCount;
	SDL_Surface *background;
	Widget **widgets;
	Label **labels;
	void (*action)(void);
	void (*returnAction)(void);
} Menu;

typedef struct BoundingBox
{
	int x, y, h, w;
} BoundingBox;

typedef struct Sprite
{
	char name[MAX_FILE_LENGTH];
	SDL_Surface *image;
	BoundingBox box;
} Sprite;

typedef struct Animation
{
	int frameCount;
	int *frameTimer, *frameID;
	int *offsetX, *offsetY;
} Animation;

typedef struct Input
{
	int left, right, up, down, add, remove;
	int previous, next, load, save, jump;
	int mouseX, mouseY, toggle;
	int activate, attack, fly, snap, cut;
	int interact, grabbing, block;
} Input;

typedef struct CustomAction
{
	int thinkTime, counter;
	void (*action)(int *, int *);
} CustomAction;

typedef struct Entity
{
	int inUse, w, h, offsetX, offsetY;
	int thinkTime, face;
	int currentFrame;
	int type, animation[MAX_ANIMATION_TYPES];
	int currentAnim, health;
	int maxHealth, damage, active, maxThinkTime;
	int targetX, targetY, environment, element;
	long flags;
	float x, y, dirX, dirY, frameSpeed, frameTimer;
	float startX, startY, endX, endY;
	float originalSpeed, speed, weight;
	char name[MAX_VALUE_LENGTH], objectiveName[MAX_VALUE_LENGTH], requires[MAX_VALUE_LENGTH];
	char description[MAX_MESSAGE_LENGTH];
	struct Entity *standingOn, *parent, *target;
	BoundingBox box;
	CustomAction customAction[MAX_CUSTOM_ACTIONS];
	void (*action)(void);
	void (*activate)(int);
	void (*draw)(void);
	void (*fallout)(void);
	void (*touch)(struct Entity *);
	void (*animationCallback)(void);
	void (*takeDamage)(struct Entity *, int);
	void (*die)(void);
	void (*reactToBlock)(void);
	void (*pain)(void);
} Entity;

typedef struct Map
{
	char filename[MAX_VALUE_LENGTH], tilesetName[MAX_VALUE_LENGTH], ambienceName[MAX_VALUE_LENGTH];
	char musicName[MAX_VALUE_LENGTH], mapName[MAX_VALUE_LENGTH];
	int startX, startY;
	int maxX, maxY, thinkTime;
	int minX, minY;
	int cameraX, cameraY;
	int cameraMinX, cameraMinY, cameraMaxX, cameraMaxY;
	int tile[MAX_MAP_Y][MAX_MAP_X];
	int hasAmbience, backgroundStartX[2], backgroundStartY[2];
	int wrapX[2], wrapY[2];
	float backgroundSpeed[2];
	Entity *targetEntity;
	SDL_Surface *background[2];
	Mix_Chunk *ambience[MAX_AMBIENT_SOUNDS];
} Map;

typedef struct Sound
{
	char name[MAX_VALUE_LENGTH];
	Mix_Chunk *effect;
} Sound;

typedef struct Game
{
	int thinkTime, weatherType, weatherThinkTime;
	int audio, hasConfig, fullscreen;
	int sfxDefaultVolume, sfxVolume, musicDefaultVolume, musicVolume;
	int showHints, status;
	int offsetX, offsetY, shakeThinkTime, shakeStrength;
	int gameType, transitionX, transitionY;
	int frames, drawScreen, paused;
	float checkpointX, checkpointY;
	long startTicks, endTicks;
	char nextMap[MAX_VALUE_LENGTH], playerStart[MAX_VALUE_LENGTH];
	void (*action)(void);
	void (*drawMenu)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	void (*transition)(void);
	void (*transitionCallback)(void);
	Menu *menu;
	SDL_Surface *screen, *tempSurface, *pauseSurface;
	TTF_Font *font;
	SDL_Joystick *joystick;
	Mix_Music *music, *bossMusic;
} Game;

typedef struct Cursor
{
	int x, y, tileID, entityType;
	int type, snapToGrid;
	Entity entity;
} Cursor;

typedef struct Message
{
	char text[MAX_MESSAGE_LENGTH];
	int thinkTime;
	SDL_Surface *surface;
	struct Message *next;
} Message;

typedef struct Inventory
{
	int selectedIndex, quantity[MAX_INVENTORY_ITEMS];
	Entity item[MAX_INVENTORY_ITEMS];
} Inventory;

typedef struct Droplet
{
	int active;
	float x, y, dirX, dirY;
} Droplet;

typedef struct Properties
{
	char name[MAX_VALUE_LENGTH], key[MAX_PROPS_ENTRIES][MAX_VALUE_LENGTH], value[MAX_PROPS_ENTRIES][MAX_VALUE_LENGTH];
	int animations[MAX_ANIMATION_TYPES];
} Properties;

typedef struct Constructor
{
	char name[MAX_VALUE_LENGTH];
	Entity *(*construct)(int, int, char *);
} Constructor;

typedef struct Target
{
	int x, y, active;
	char name[MAX_VALUE_LENGTH];
} Target;

typedef struct Type
{
	int id;
	char name[MAX_VALUE_LENGTH];
} Type;

typedef struct Trigger
{
	char triggerName[MAX_VALUE_LENGTH], targetName[MAX_VALUE_LENGTH];
	int inUse, targetType, count;
} Trigger;

typedef struct Objective
{
	int inUse, active, completed;
	char name[MAX_MESSAGE_LENGTH], completionTrigger[MAX_VALUE_LENGTH];
} Objective;

typedef struct Script
{
	int line, lineCount, skipping, thinkTime, counter;
	char **text;
} Script;

typedef struct Hud
{
	int thinkTime;
	SDL_Surface *itemBox, *heart, *emptyHeart;
	Message infoMessage;
} Hud;

typedef struct Control
{
	int button[MAX_CONTROLS];
} Control;

typedef struct EntityList
{
	Entity *entity;
	struct EntityList *next;
} EntityList;

typedef struct Grid
{
	int count;
	EntityList listHead;
	struct Grid *next;
} Grid;

typedef struct FileData
{
	char filename[MAX_FILE_LENGTH];
	long fileSize, compressedSize, offset;
} FileData;
