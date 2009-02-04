#include "defs.h"

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
	int interact;
} Input;

typedef struct Entity
{
	int inUse, w, h, offsetX, offsetY;
	int thinkTime, face;
	int currentFrame, frameTimer, frameSpeed;
	int type, animation[MAX_ANIMATION_TYPES];
	int currentAnim, health, customThinkTime[MAX_CUSTOM_ACTIONS];
	int maxHealth, damage, active;
	long flags;
	float x, y, dirX, dirY;
	float startX, startY, endX, endY;
	float targetX, targetY, speed;
	char name[MAX_VALUE_LENGTH], objectiveName[MAX_VALUE_LENGTH], requires[MAX_VALUE_LENGTH];
	struct Entity *standingOn, *parent, *target;
	void (*action)(void);
	void (*activate)(int);
	void (*draw)(void);
	void (*touch)(struct Entity *);
	void (*animationCallback)(void);
	void (*custom[MAX_CUSTOM_ACTIONS])(int *);
	void (*takeDamage)(struct Entity *, int);
	void (*die)(void);
} Entity;

typedef struct Map
{
	char filename[MAX_VALUE_LENGTH], tilesetName[MAX_VALUE_LENGTH], ambienceName[MAX_VALUE_LENGTH];
	char musicName[MAX_VALUE_LENGTH];
	int startX, startY;
	int maxX, maxY, thinkTime;
	int tile[MAX_MAP_Y][MAX_MAP_X];
	int hasAmbience;
	Entity *targetEntity;
	SDL_Surface *background;
	Mix_Chunk *ambience[MAX_AMBIENT_SOUNDS];
} Map;

typedef struct Sound
{
	char name[MAX_VALUE_LENGTH];
	Mix_Chunk *effect;
} Sound;

typedef struct Game
{
	char sword[MAX_VALUE_LENGTH], shield[MAX_VALUE_LENGTH];
	int thinkTime, weatherType, weatherThinkTime;
	int audio, audioVolume;
	int offsetX, offsetY, shakeThinkTime, shakeStrength;
	void (*action)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	SDL_Surface *screen;
	TTF_Font *font;
	Mix_Music *music;
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
	int thinkTime, inUse;
} Message;

typedef struct Inventory
{
	int selectedIndex;
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
	Entity *(*construct)(int, int);
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
