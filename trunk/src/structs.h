#include "defs.h"

typedef struct BoundingBox
{
	int x, y, h, w;
} BoundingBox;

typedef struct Sprite
{
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
	int mouseX, mouseY, toggle, drop;
	int activate, attack, fly, snap, cut;
	int interact, grabbing;
} Input;

typedef struct Entity
{
	int inUse, w, h, offsetX, offsetY;
	int thinkTime, face;
	int currentFrame, frameTimer, frameSpeed;
	int type, animation[MAX_ANIMATION_TYPES];
	int currentAnim, health, customThinkTime[MAX_CUSTOM_ACTIONS];
	int maxHealth, damage, active, maxThinkTime;
	int targetX, targetY, environment;
	long flags;
	float x, y, dirX, dirY;
	float startX, startY, endX, endY;
	float speed, weight;
	char name[MAX_VALUE_LENGTH], objectiveName[MAX_VALUE_LENGTH], requires[MAX_VALUE_LENGTH];
	char description[MAX_MESSAGE_LENGTH];
	struct Entity *standingOn, *parent, *target;
	BoundingBox box;
	void (*action)(void);
	void (*activate)(int);
	void (*draw)(void);
	void (*fallout)(void);
	void (*touch)(struct Entity *);
	void (*animationCallback)(void);
	void (*custom[MAX_CUSTOM_ACTIONS])(int *);
	void (*takeDamage)(struct Entity *, int);
	void (*die)(void);
	void (*pain)(void);
} Entity;

typedef struct Map
{
	char filename[MAX_VALUE_LENGTH], tilesetName[MAX_VALUE_LENGTH], ambienceName[MAX_VALUE_LENGTH];
	char musicName[MAX_VALUE_LENGTH], mapName[MAX_VALUE_LENGTH];
	int startX, startY;
	int maxX, maxY, thinkTime;
	int minX, minY;
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
	int audio, audioVolume;
	int offsetX, offsetY, shakeThinkTime, shakeStrength;
	int gameType, transitionX, transitionY;
	int frames, drawScreen, paused;
	float checkpointX, checkpointY;
	long startTicks, endTicks;
	char nextMap[MAX_VALUE_LENGTH], playerStart[MAX_VALUE_LENGTH];
	void (*action)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	void (*transition)(void);
	void (*transitionCallback)(void);
	SDL_Surface *screen, *tempSurface;
	TTF_Font *font;
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
	int thinkTime, inUse, type;
	SDL_Surface *surface;
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
	int line, lineCount, skipping;
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
