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

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAX_MAP_X 1200
#define MAX_MAP_Y 600

#define GRID_COUNT 60

#define GRID_WIDTH 20
#define GRID_HEIGHT 10

#define TILE_SIZE 32

#define PI 3.14159265

#define GRAVITY_SPEED 0.75

#define MAX_AIR_SPEED 15

#define MAX_WATER_SPEED 0.5

#define JUMP_HEIGHT 12

#define TRANS_R 127
#define TRANS_G 0
#define TRANS_B 127

#define MAX_ANIMATIONS 255

#define MAX_ENTITIES 1024

#define MAX_DIALOG_WIDTH 480

#define BLANK_TILE 0

#define SOLID_TILE_START 1

#define SOLID_TILE_END 99

#define SLOPE_UP_START 100

#define SLOPE_UP_END 104

#define SLOPE_DOWN_START 105

#define SLOPE_DOWN_END 109

#define JUMP_THROUGH_TILE_START 110

#define JUMP_THROUGH_TILE_END 119

#define BACKGROUND_TILE_START 120

#define BACKGROUND_TILE_END 300

#define FOREGROUND_TILE_START 301

#define WATER_TILE_START 496

#define WATER_TILE_END 503

#define LAVA_TILE_START 504

#define LAVA_TILE_END 511

#define SLIME_TILE_START 512

#define SLIME_TILE_END 519

#define MAX_TILES 520

#define MAX_LINE_LENGTH 1024

#define MAX_PATH_LENGTH 1024

#define MAX_FILE_LENGTH 256

#define PI 3.14159265

#define MAX_MESSAGE_LENGTH 256

#define MAX_INVENTORY_ITEMS 25

#define MAX_DROPS 240

#define MAX_SPRITES 4096

#define MAX_PROPS_ENTRIES 20

#define MAX_PROPS_FILES 256

#define MAX_CUSTOM_ACTIONS 4

#define MAX_AMBIENT_SOUNDS 10

#define MAX_DECORATIONS 100

#define MAX_TARGETS 255

#define MAX_HUD_MESSAGES 5

#define MAX_VALUE_LENGTH 60

#define MAX_TRIGGERS 10

#define MAX_OBJECTIVES 5

#define MAX_INPUTS 256

#define MAX_ITEMS_IN_SLOT 10

#define MAX_SAVE_SLOTS 10

#define MAX_STACKABLES 20

#ifdef WIN32
	#define strcmpignorecase(x, y) stricmp(x, y)
#else
	#define strcmpignorecase(x, y) strcasecmp(x, y)
#endif

#define INVENTORY_BOX_SIZE 42

#define INVENTORY_BOX_COUNT 5

#define DEG_TO_RAD(x) (x * PI / 180)

#define STRNCPY(dest, src, n) strncpy(dest, src, n); dest[n - 1] = '\0'

#if DEV == 1
	#ifdef INSTALL_PATH
		#undef INSTALL_PATH
	#endif

	#ifdef PAK_FILE
		#undef PAK_FILE
	#endif

	#define INSTALL_PATH ""

	#define PAK_FILE "data.pak"
#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	#define SWAP16(X) (X)
	#define SWAP32(X) (X)
#else
	#define SWAP16(X) SDL_Swap16(X)
	#define SWAP32(X) SDL_Swap32(X)
#endif

#define MAX_SOUNDS 256

#define ITEM_JUMP_HEIGHT -6

#define _(string) gettext(string)

#define SCREENSHOT_KEY SDLK_F10

#define FULLSCREEN_KEY SDLK_F12

#define VOLUME_STEPS 12.8

#define SIGN(x) ((x) > 0 ? 1 : ((x) == 0 ? 0 : (-1)))

enum
{
	LEFT,
	RIGHT
};

enum
{
	FALSE,
	TRUE
};

enum
{
	ON_GROUND = 1,
	NO_DRAW = 2,
	PUSHABLE = 4,
	HELPLESS = 8,
	INVULNERABLE = 16,
	FLY = 32,
	ATTACKING = 64,
	BLOCKING = 128,
	DO_NOT_PERSIST = 256,
	GRABBING = 512,
	ATTACK_SUCCESS = 1024,
	STACKABLE = 2048,
	OBSTACLE = 4096,
	FLOATS = 8192,
	FLASH = 16384,
	UNBLOCKABLE = 32768,
	BOUNCES = 65536,
	GRABBED = 131072,
	PLAYER_TOUCH_ONLY = 262144
};

enum
{
	NO_ELEMENT,
	FIRE,
	ICE,
	LIGHTNING,
	PHANTASMAL
};

enum
{
	PLAYER,
	WEAPON,
	ITEM,
	KEY_ITEM,
	ENEMY,
	HEALTH,
	SHIELD,
	AUTO_LIFT,
	MANUAL_LIFT,
	TARGET,
	SPAWNER,
	PRESSURE_PLATE,
	MANUAL_DOOR,
	AUTO_DOOR,
	WEAK_WALL,
	SWITCH,
	LINE_DEF,
	LEVEL_EXIT,
	PROJECTILE,
	SAVE_POINT,
	TEMP_ITEM,
	NPC,
	ACTION_POINT,
	SCRIPT_LINE_DEF,
	FALLING_PLATFORM,
	TRAP_DOOR
};

enum
{
	TILES,
	ENTITIES
};

enum
{
	STAND,
	WALK,
	JUMP,
	PAIN,
	DIE,
	BLOCK,
	ATTACK_1,
	ATTACK_2,
	ATTACK_3,
	ATTACK_4,
	ATTACK_5,
	CUSTOM_1,
	CUSTOM_2,
	CUSTOM_3,
	CUSTOM_4,
	CUSTOM_5,
	TAUNT,
	MAX_ANIMATION_TYPES
};

enum
{
	NO_WEATHER,
	LIGHT_RAIN,
	HEAVY_RAIN,
	STORMY,
	SNOW,
	MAX_WEATHER_TYPES
};

enum
{
	EDGAR_CHANNEL,
	BOSS_CHANNEL
};

enum
{
	LIGHT = 1,
	MEDIUM,
	STRONG
};

enum
{
	UPDATE_OBJECTIVE = 1,
	ACTIVATE_ENTITY,
	RUN_SCRIPT,
	UPDATE_TRIGGER
};

enum
{
	NORMAL,
	RECORDING,
	REPLAYING
};

enum
{
	WIPE_OUT_LEFT_TO_RIGHT,
	WIPE_OUT_RIGHT_TO_LEFT,
	WIPE_OUT_CIRCLE_TO_LARGE,
	WIPE_OUT_CIRCLE_TO_SMALL,
	MAX_OUT_TRANSITIONS
};

enum
{
	WIPE_IN_LEFT_TO_RIGHT,
	WIPE_IN_RIGHT_TO_LEFT,
	WIPE_IN_CIRCLE_TO_LARGE,
	WIPE_IN_CIRCLE_TO_SMALL,
	MAX_IN_TRANSITIONS
};

enum
{
	TRANSITION_IN,
	TRANSITION_OUT
};

enum
{
	AIR,
	WATER,
	LAVA,
	SLIME
};

enum
{
	PLAYER_INVENTORY,
	ENTITY_DATA
};

enum
{
	CONTROL_LEFT,
	CONTROL_RIGHT,
	CONTROL_UP,
	CONTROL_DOWN,
	CONTROL_JUMP,
	CONTROL_ADD,
	CONTROL_NEXT,
	CONTROL_PREVIOUS,
	CONTROL_SAVE,
	CONTROL_LOAD,
	CONTROL_TOGGLE,
	CONTROL_DROP,
	CONTROL_ACTIVATE,
	CONTROL_CUT,
	CONTROL_ATTACK,
	CONTROL_FLY,
	CONTROL_SNAP,
	CONTROL_INTERACT,
	CONTROL_GRABBING,
	CONTROL_PAUSE,
	CONTROL_BLOCK,
	CONTROL_INVENTORY,
	MAX_CONTROLS
};

enum
{
	IN_TITLE,
	IN_MENU,
	IN_GAME,
	IN_INVENTORY,
	IN_EDITOR
};

enum
{
	BACKGROUND_LAYER,
	MID_GROUND_LAYER,
	FOREGROUND_LAYER
};
