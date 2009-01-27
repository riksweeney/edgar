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
} Input;

typedef struct Entity
{
	int active, w, h, offsetX, offsetY;
	int thinkTime, face;
	int currentFrame, frameTimer;
	int type, animation[MAX_ANIMATION_TYPES];
	int currentAnim, health, customThinkTime[MAX_CUSTOM_ACTIONS];
	int maxHealth, damage;
	long flags;
	float x, y, dirX, dirY;
	float startX, startY, endX, endY;
	float targetX, targetY, speed;
	char name[30], objectiveName[30], activates[30];
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
	char filename[30], tilesetName[30], ambienceName[30];
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
	char name[255];
	Mix_Chunk *effect;
} Sound;

typedef struct Game
{
	char sword[30], shield[30];
	int thinkTime, weatherType, weatherThinkTime;
	int audio, audioVolume;
	void (*action)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	SDL_Surface *screen;
	TTF_Font *font;
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
	int counter;
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
	char name[30], key[MAX_PROPS_ENTRIES][30], value[MAX_PROPS_ENTRIES][60];
	int animations[MAX_ANIMATION_TYPES];
} Properties;

typedef struct Constructor
{
	char name[30];
	Entity *(*construct)(int, int);
} Constructor;

typedef struct Target
{
	int x, y, active;
	char name[30];
} Target;
