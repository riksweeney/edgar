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

#include "defs.h"

typedef struct Texture
{
	int w, h;
	SDL_Texture *texture;
} Texture;

typedef struct TextureCache
{
	char name[MAX_MESSAGE_LENGTH];
	long expiry;
	Texture *texture;
	struct TextureCache *next;
} TextureCache;

typedef struct Colour
{
	int r, g, b, a;
} Colour;

typedef struct Label
{
	int x, y;
	int r, g, b;
	Texture *text;
} Label;

typedef struct Widget
{
	int x, y, disabled, hidden;
	int *value, maxValue, minValue;
	int r, g, b;
	void (*clickAction)(void);
	void (*rightAction)(void);
	void (*leftAction)(void);
	Label *label;
	Texture *normalState, *selectedState, *disabledState;
} Widget;

typedef struct Menu
{
	int index, x, y, w, h, widgetCount;
	int startY, endY;
	Texture *background;
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
	int w, h;
	Texture *image;
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
	int activate, attack, snap, cut;
	int interact, grabbing, block, inventory;
	int lastPressedKey, xAxisMoved, yAxisMoved;
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
	unsigned long flags;
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
	int wrapX[2], wrapY[2];
	float backgroundSpeed[2], cameraSpeed;
	float cameraX, cameraY;
	AnimTile *animTile;
	Entity *targetEntity;
	Texture *background[2];
	Mix_Chunk *ambience[MAX_AMBIENT_SOUNDS];
} Map;

typedef struct Sound
{
	char name[MAX_VALUE_LENGTH];
	Mix_Chunk *effect;
} Sound;

typedef struct AlphaSurface
{
	int x, y, w, h;
	int r, g, b;
} AlphaSurface;

typedef struct Game
{
	int thinkTime, weatherType, weatherThinkTime;
	int audio, hasConfig, fullscreen, gameOverX;
	int sfxDefaultVolume, musicDefaultVolume;
	int showHints, status, fps, audioQuality;
	int offsetX, offsetY, shakeThinkTime, shakeStrength;
	int gameType, transitionX, transitionY, previousStatus;
	int frames, drawScreen, paused, firstRun, audioDisabled;
	int kills, batsDrowned, timesEaten, cheating;
	int attacksBlocked, timeSpentAsSlime, arrowsFired;
	int secretsFound, alphaTime, canContinue, continues;
	int disableJoystick, mapExitable, cheatsEnabled;
	int infiniteEnergy, infiniteArrows, lavaIsFatal;
	int saveOnExit, overrideMusic, showHUD;
	int fontSizeSmall, fontSizeLarge;
	unsigned int distanceTravelled;
	float checkpointX, checkpointY;
	long startTicks, endTicks, playTime;
	char nextMap[MAX_VALUE_LENGTH], playerStart[MAX_VALUE_LENGTH];
	char customFont[MAX_FILE_LENGTH];
	void (*drawMenu)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	void (*transition)(void);
	void (*transitionCallback)(void);
	Menu *menu;
	AlphaSurface alphaSurface;
	SDL_Renderer *renderer;
	SDL_Window *window;
	Texture *screen, *gameOverSurface, *tempSurface, *pauseSurface;
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
	Texture *surface;
	struct Message *next;
} Message;

typedef struct Inventory
{
	int selectedIndex, x, y, cursorIndex;
	int hasBow, hasLightningSword;
	Entity item[MAX_INVENTORY_ITEMS];
	Texture *background, *cursor, *description, *objectives;
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
	Texture *itemBox, *heart, *whiteHeart, *emptyHeart, *spotlight, *medalTextSurface, *quantitySurface;
	Texture *medalSurface[4], *disabledMedalSurface;
	Texture *slimeTimerSurface;
	Message infoMessage;
} Hud;

typedef struct MedalQueue
{
	int processing, thinkTime;
	Message medalMessage;
} MedalQueue;

typedef struct Medal
{
	char code[MAX_VALUE_LENGTH], description[MAX_MESSAGE_LENGTH];
	int medalType, obtained, hidden;
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
	int32_t fileSize, compressedSize, offset;
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
	Texture *dialogSurface;
} DialogBox;

typedef struct CreditLine
{
	char text[MAX_LINE_LENGTH];
	int r, g, b;
	float x, y;
	Texture *textImage;
} CreditLine;

typedef struct Credits
{
	int line, lineCount, entityID, fading, alpha;
	int startDelay, nextEntityDelay, status;
	float speed;
	CreditLine *creditLine;
	AlphaSurface fadeSurface;
	Texture *edgarLogo, *prLogo;
} Credits;

typedef struct Title
{
	int thinkTime, continueSlot;
	Texture *edgarLogo, *copyright, *startButton;
} Title;

typedef struct Bucket
{
	char *key, *value;
	struct Bucket *next;
} Bucket;

typedef struct HashTable
{
	Bucket **bucket;
	int *bucketCount;
} HashTable;

typedef struct MOHeader
{
	int32_t magicNumber, version, stringCount;
	int32_t originalOffset, translationOffset;
} MOHeader;

typedef struct MOEntry
{
	int32_t length, offset;
} MOEntry;
