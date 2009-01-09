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
	int activate, attack, fly;
} Input;

typedef struct Entity
{
	int active, w, h, offsetX, offsetY;
	int thinkTime, face;
	int currentFrame, frameTimer;
	int type, animation[MAX_ENTITY_ANIMATIONS];
	int sound[MAX_ENTITY_SOUNDS];
	int currentAnim, health, customThinkTime[MAX_CUSTOM_ACTIONS];
	int maxHealth, damage;
	long flags;
	float x, y, dirX, dirY;
	float startX, startY, endX, endY;
	float targetX, targetY, speed;
	char name[30], objectiveName[30], activates[30];
	struct Entity *standingOn, *parent, *target;
	void (*action)(void);
	void (*activate)(void);
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
	Mix_Chunk *effect;
} Sound;

typedef struct Game
{
	char sword[30], shield[30];
	int thinkTime, weatherType, weatherThinkTime;
	int audio;
	void (*action)(void);
	void (*weatherAction)(void);
	void (*weatherDraw)(void);
	SDL_Surface *screen;
	TTF_Font *font;
} Game;

typedef struct Cursor
{
	int x, y, tileID, entityType;
	int type;
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
	char name[30], key[MAX_PROPS_ENTRIES][30], value[MAX_PROPS_ENTRIES][30];
} Properties;

typedef struct Enemy
{
	char name[30];
	void (*construct)(int, int);
} Enemy;
