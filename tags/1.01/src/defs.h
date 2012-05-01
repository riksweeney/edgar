/*
Copyright (C) 2009-2012 Parallel Realities

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

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAX_MAP_X 1200
#define MAX_MAP_Y 600

#define GRID_SIZE 15

#define GRID_MAX_X 80
#define GRID_MAX_Y 40

#define TILE_SIZE 32

#define PI 3.14159265

#define GRAVITY_SPEED 0.75

#define MAX_AIR_SPEED 15

#define MAX_WATER_SPEED 0.5

#define JUMP_HEIGHT 12

#define TRANS_R 127
#define TRANS_G 255
#define TRANS_B 127

#define MAX_ANIMATIONS 1024

#define MAX_ENTITIES 1024

#define MAX_DIALOG_WIDTH 480

#define MAX_SCRIPT_WIDTH 380

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

#define SLIME_TILE_BLEND 520

#define SLIME_TILE_BLEND_REVERSE 521

#define MAX_TILES 522

#define MAX_LINE_LENGTH 1024

#define MAX_PATH_LENGTH 1024

#define MAX_FILE_LENGTH 256

#define MAX_MESSAGE_LENGTH 256

#define MAX_DROPS 256

#define MAX_SPRITES 4096

#define MAX_PROPS_ENTRIES 20

#define MAX_PROPS_FILES 256

#define MAX_CUSTOM_ACTIONS 4

#define MAX_AMBIENT_SOUNDS 10

#define MAX_TARGETS 255

#define MAX_HUD_MESSAGES 5

#define MAX_VALUE_LENGTH 80

#define MAX_OBJECTIVES 25

#define MAX_INPUTS 256

#define MAX_SAVE_SLOTS 10

#define MAX_STACKABLES 20

#define TELEPORT_SPEED 10

#define SPAWNED_IN_TIME 18000

#ifdef _WIN32
	#define strcmpignorecase(x, y) stricmp(x, y)
#else
	#define strcmpignorecase(x, y) strcasecmp(x, y)
#endif

#define INVENTORY_BOX_SIZE 42

#define INVENTORY_ROW_COUNT 5

#define INVENTORY_COLUMN_COUNT 6

#define MAX_INVENTORY_ITEMS 30

#define MAX_TRIGGERS MAX_INVENTORY_ITEMS

#define DEG_TO_RAD(x) ((x) * PI / 180)

#define TOTAL_SECRETS 26

#define STRNCPY(dest, src, n) strncpy(dest, src, n); dest[n - 1] = '\0'

#define SWORD_MAX_CHARGE 50

#if DEV == 1
	#ifdef INSTALL_PATH
		#undef INSTALL_PATH
	#endif

	#ifdef LOCALE_DIR
		#undef LOCALE_DIR
	#endif

	#ifdef PAK_FILE
		#undef PAK_FILE
	#endif

	#define INSTALL_PATH ""

	#define LOCALE_DIR "locale/"

	#define PAK_FILE "data.pak"
#elif DEV != 0
	#error "DEV must be 0 or 1"
#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	#define SWAP16(X) (X)
	#define SWAP32(X) (X)
	#define SWAP64(X) (X)
#else
	#define SWAP16(X) SDL_Swap16(X)
	#define SWAP32(X) SDL_Swap32(X)
	#define SWAP64(X) SDL_Swap64(X)
#endif

#define MAX_SOUNDS 256

#define ITEM_JUMP_HEIGHT -6

#define _(string) getTranslatedString(string)

#define SCREENSHOT_KEY SDLK_F10

#define FULLSCREEN_KEY SDLK_F12

#define VOLUME_STEPS 12.8

#define SIGN(x) ((x) > 0 ? 1 : ((x) == 0 ? 0 : (-1)))

#define MAX_CHANNELS 16

#define NORMAL_FONT_SIZE 14

#define LARGE_FONT_SIZE 26

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define HIT_INVULNERABLE_TIME 20

#define MENU_CODE "fasterthanthespeedoflove"

#define MAX_ANIMATION_TYPES 50

#define rand() USE_PRAND_INSTEAD_OF_RAND()

#define BORDER_PADDING 5

#define BUTTON_PADDING 20

#define TABLE_SIZE 255

#ifdef TRUE
	#undef TRUE
#endif

#ifdef FALSE
	#undef FALSE
#endif

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
	PLAYER_TOUCH_ONLY = 262144,
	TELEPORTING = 524288,
	NO_END_TELEPORT_SOUND = 1048576,
	ATTRACTED = 2097152,
	SPAWNED_IN = 4194304,
	WRAPPED = 8388608,
	LIMIT_TO_SCREEN = 16777216,
	WAS_STANDING_ON = 33554432,
	GROUNDED = 67108864,
	FRICTIONLESS = 134217728,
	CONFUSED = 268435456
};

enum
{
	NO_ELEMENT,
	FIRE,
	ICE,
	LIGHTNING,
	PHANTASMAL,
	DRAGON_FIRE
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
	TRAP_DOOR,
	CONVEYOR_BELT,
	TELEPORTER,
	VANISHING_PLATFORM,
	ANTI_GRAVITY,
	EXPLOSION
};

enum
{
	TILES,
	ENTITIES
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
	UPDATE_TRIGGER,
	KILL_ENTITY,
	REMOVE_INVENTORY_ITEM,
	DEACTIVATE_ENTITY,
	UPDATE_EXIT
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
	IN_EDITOR,
	IN_ERROR,
	IN_CREDITS
};

enum
{
	BACKGROUND_LAYER,
	MID_GROUND_LAYER,
	FOREGROUND_LAYER,
	MAX_LAYERS
};
