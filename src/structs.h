/*
Copyright (C) 2009-2011 Parallel Realities

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
	int x, y, disabled, hidden;
	int *value, maxValue, minValue;
	void (*clickAction)(void);
	void (*rightAction)(void);
	void (*leftAction)(void);
	Label *label;
	SDL_Surface *normalState, *selectedState, *disabledState;
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
	int interact, grabbing, block, inventory;
	int lastPressedKey;
} Input;

typedef struct CustomAction
{
	int thinkTime, counter;
	float value;
	void (*action)(int *, int *, float *);
} CustomAction;

typedef struct EntityAnimation
{
	char name[MAX_VALUE_LENGTH];
	int id;
} EntityAnimation;

typedef struct Entity
{
	int inUse, w, h, offsetX, offsetY;
	int thinkTime, face, layer;
	int currentFrame;
	int type;
	int currentAnim, health, alpha;
	int maxHealth, damage, active, maxThinkTime;
	int targetX, targetY, environment, element;
	int mental, spawnTime;
	long flags;
	float x, y, dirX, dirY, frameSpeed, frameTimer;
	float startX, startY, endX, endY;
	float originalSpeed, speed, weight, originalWeight;
	char name[MAX_VALUE_LENGTH], objectiveName[MAX_VALUE_LENGTH], requires[MAX_VALUE_LENGTH];
	char description[MAX_MESSAGE_LENGTH], animationName[MAX_VALUE_LENGTH];
	EntityAnimation animation[MAX_ANIMATION_TYPES];
	struct Entity *standingOn, *parent, *target, *head;
	BoundingBox box;
	CustomAction customAction[MAX_CUSTOM_ACTIONS];
	void (*action)(void);
	void (*activate)(int);
	int (*draw)(void);
	void (*fallout)(void);
	void (*touch)(struct Entity *);
	void (*animationCallback)(void);
	void (*takeDamage)(struct Entity *, int);
	void (*die)(void);
	void (*reactToBlock)(struct Entity *);
	void (*pain)(void);
	void (*resumeNormalFunction)(void);
	void (*creditsAction)(void);
} Entity;

typedef struct AnimTile
{
	int tileCount, tileIndex;
	int *tile;
} AnimTile;

typedef struct Map
{
	char filename[MAX_VALUE_LENGTH], tilesetName[MAX_VALUE_LENGTH], ambienceName[MAX_VALUE_LENGTH];
	char musicName[MAX_VALUE_LENGTH], mapName[MAX_VALUE_LENGTH];
	int startX, startY;
	int maxX, maxY, thinkTime;
	int minX, minY, blendTime;
	int forceMinY, darkMap;
	int animTileTotal, animThinkTime;
	int cameraMinX, cameraMinY, cameraMaxX, cameraMaxY;
	int playerMinX, playerMaxX;
	int tile[MAX_MAP_Y][MAX_MAP_X];
	int hasAmbience, backgroundStartX[2], backgroundStartY[2];
	int wrapX[2], wrapY[2], waterBackgroundTile;
	float backgroundSpeed[2], cameraSpeed;
	float cameraX, cameraY;
	AnimTile *animTile;
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
	int showHints, status, fps, audioQuality;
	int offsetX, offsetY, shakeThinkTime, shakeStrength;
	int gameType, transitionX, transitionY;
	int frames, drawScreen, paused, firstRun, audioDisabled;
	int kills, batsDrowned, timesEaten, medalSupport;
	int attacksBlocked, timeSpentAsSlime, arrowsFired;
	int secretsFound, alphaTime, canContinue, continues;
	int disableJoystick, mapExitable, cheatsEnabled;
	int infiniteEnergy, infiniteArrows, cheating;
	int saveOnExit;
	unsigned int distanceTravelled;
	float checkpointX, checkpointY;
	long startTicks, endTicks, playTime;
	char nextMap[MAX_VALUE_LENGTH], playerStart[MAX_VALUE_LENGTH];
	void (*drawMenu)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	void (*transition)(void);
	void (*transitionCallback)(void);
	Menu *menu;
	SDL_Surface *screen, *tempSurface, *pauseSurface, *gameOverSurface, *alphaSurface;
	TTF_Font *font, *largeFont;
	SDL_Joystick *joystick;
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
	int thinkTime, r, g, b;
	SDL_Surface *surface;
	struct Message *next;
} Message;

typedef struct Inventory
{
	int selectedIndex, x, y, cursorIndex;
	int hasBow, hasLightningSword;
	Entity item[MAX_INVENTORY_ITEMS];
	SDL_Surface *background, *cursor, *description, *objectives;
} Inventory;

typedef struct Droplet
{
	int active;
	float x, y, dirX, dirY;
} Droplet;

typedef struct Properties
{
	char name[MAX_VALUE_LENGTH], key[MAX_PROPS_ENTRIES][MAX_VALUE_LENGTH], value[MAX_PROPS_ENTRIES][MAX_VALUE_LENGTH];
	EntityAnimation animations[MAX_ANIMATION_TYPES];
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
	int inUse, targetType, count, total;
} Trigger;

typedef struct Objective
{
	int inUse, active, completed;
	char name[MAX_MESSAGE_LENGTH], completionTrigger[MAX_VALUE_LENGTH];
} Objective;

typedef struct Script
{
	int line, lineCount, skipping, thinkTime, counter;
	int requiredDepth, currentDepth, yesNoResult;
	char **text;
	Menu *menu;
	void (*draw)(void);
} Script;

typedef struct Hud
{
	int thinkTime, *bossHealth, bossHealthIndex, bossMaxHealth, medalThinkTime, quantity;
	SDL_Surface *itemBox, *heart, *emptyHeart, *spotlight, *medalTextSurface, *quantitySurface;
	SDL_Surface *medalSurface[4];
	Message infoMessage;
} Hud;

typedef struct Medal
{
	char privateKey[MAX_VALUE_LENGTH];
	int processing, connected, privateKeyFound;
	Message medalMessage;
	IPaddress ip;
	SDL_Thread *thread;
} Medal;

typedef struct Control
{
	int deadZone;
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

typedef struct ContinueData
{
	char boss[MAX_VALUE_LENGTH];
	int cameraMinX, cameraMinY, cameraMaxX, cameraMaxY;
	int cameraFollow;
	void (*resumeAction)(void);
} ContinueData;

typedef struct DialogBox
{
	int thinkTime;
	SDL_Surface *dialogSurface;
} DialogBox;

typedef struct CreditLine
{
	char text[MAX_LINE_LENGTH];
	int r, g, b;
	float x, y;
	SDL_Surface *textImage;
} CreditLine;

typedef struct Credits
{
	int line, lineCount, entityID, fading, alpha;
	int startDelay, nextEntityDelay, status;
	CreditLine *creditLine;
	SDL_Surface *edgarLogo, *prLogo, *fadeSurface;
} Credits;

