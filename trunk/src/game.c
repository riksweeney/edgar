#include "headers.h"

#include "game.h"
#include "weather.h"
#include "graphics/font.h"
#include "system/random.h"
#include "audio/music.h"
#include "graphics/graphics.h"
#include "system/resources.h"
#include "map.h"
#include "system/load_save.h"
#include "entity.h"
#include "player.h"
#include "menu/menu.h"
#include "menu/main_menu.h"

extern Game game;

static void shake(void);
static void wipeOutRightToLeft(void);
static void wipeOutLeftToRight(void);
static void wipeInRightToLeft(void);
static void wipeInLeftToRight(void);
static void wipeInCircleToLarge(void);
static void wipeInCircleToSmall(void);
static void wipeOutCircleToLarge(void);
static void wipeOutCircleToSmall(void);

void initGame()
{
	game.weatherType = 0;

	game.drawScreen = TRUE;

	game.paused = FALSE;

	game.showHints = FALSE;

	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;

	setWeather(game.weatherType);
}

void doGame()
{
	/* Decrease game's thinktime */

	game.thinkTime--;

	if (game.thinkTime <= 0)
	{
		/* Execute the action if there is one */

		if (game.action != NULL)
		{
			game.action();
		}

		game.thinkTime = 0;
	}

	if (game.shakeThinkTime > 0)
	{
		shake();

		game.shakeThinkTime--;

		if (game.shakeThinkTime <= 0)
		{
			game.offsetX = game.offsetY = 0;
		}
	}

	if (game.weatherType != NO_WEATHER)
	{
		game.weatherAction();
	}
}

void freeGame()
{
	if (game.pauseSurface != NULL)
	{
		SDL_FreeSurface(game.pauseSurface);

		game.pauseSurface = NULL;
	}
}

void drawGame()
{
	if (game.weatherType != NO_WEATHER)
	{
		game.weatherDraw();
	}

	if (game.transition != NULL)
	{
		game.transition();
	}

}

void setTransition(int type, void (*func)(void))
{
	if (type == TRANSITION_OUT)
	{
		switch (prand() % MAX_OUT_TRANSITIONS)
		{
			case WIPE_OUT_RIGHT_TO_LEFT:
				game.transitionX = SCREEN_WIDTH;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeOutRightToLeft;
			break;

			case WIPE_OUT_CIRCLE_TO_LARGE:
				game.transitionX = 0;
				game.transition = &wipeOutCircleToLarge;
			break;

			case WIPE_OUT_CIRCLE_TO_SMALL:
				game.transitionX = SCREEN_WIDTH;
				game.transition = &wipeOutCircleToSmall;
			break;

			default:
				game.transitionX = 0;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeOutLeftToRight;
			break;
		}
	}

	else
	{
		switch (prand() % MAX_IN_TRANSITIONS)
		{
			case WIPE_IN_RIGHT_TO_LEFT:
				game.transitionX = SCREEN_WIDTH;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeInRightToLeft;
			break;

			case WIPE_IN_CIRCLE_TO_LARGE:
				game.transitionX = 0;
				game.transition = &wipeInCircleToLarge;
			break;

			case WIPE_IN_CIRCLE_TO_SMALL:
				game.transitionX = SCREEN_WIDTH;
				game.transition = &wipeInCircleToSmall;
			break;

			default:
				game.transitionX = 0;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeInLeftToRight;
			break;
		}
	}

	game.transitionCallback = func;
}

void shakeScreen(int shakeStrength, int time)
{
	game.shakeThinkTime = time;
	game.shakeStrength = shakeStrength;
}

static void shake()
{
	switch (game.shakeStrength)
	{
		case LIGHT:
			game.offsetX = prand() % 2 * (prand() % 2 == 0 ? -1 : 1);
			game.offsetY = prand() % 2 * (prand() % 2 == 0 ? -1 : 1);
		break;

		case MEDIUM:
			game.offsetX = prand() % 4 * (prand() % 2 == 0 ? -1 : 1);
			game.offsetY = prand() % 4 * (prand() % 2 == 0 ? -1 : 1);
		break;

		case STRONG:
			game.offsetX = prand() % 6 * (prand() % 2 == 0 ? -1 : 1);
			game.offsetY = prand() % 6 * (prand() % 2 == 0 ? -1 : 1);
		break;
	}
}

static void wipeOutRightToLeft()
{
	adjustMusicVolume(-1);

	if (game.transitionX <= -15)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(game.transitionX < 0 ? 0 : game.transitionX, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX -= 15;
}

static void wipeOutLeftToRight()
{
	adjustMusicVolume(-1);

	if (game.transitionX > SCREEN_WIDTH + 15)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX += 15;
}

static void wipeInLeftToRight()
{
	if (game.transitionX >= SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(game.transitionX, 0, SCREEN_WIDTH - game.transitionX, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX += 15;
}

static void wipeInRightToLeft()
{
	if (game.transitionX < 0)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX -= 15;
}

static void wipeInCircleToLarge()
{
	if (game.transitionX > SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircleFromSurface(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX);

	game.transitionX += 10;
}

static void wipeInCircleToSmall()
{
	if (game.transitionX < 0)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX, 0, 0, 0);

	game.transitionX -= 10;
}

static void wipeOutCircleToLarge()
{
	if (game.transitionX > SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX, 0, 0, 0);

	game.transitionX += 10;
}

static void wipeOutCircleToSmall()
{
	if (game.transitionX <= -10)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircleFromSurface(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX < 0 ? 0 : game.transitionX);

	game.transitionX -= 10;
}

void setNextLevel(char *name, char *playerStart)
{
	STRNCPY(game.nextMap, name, sizeof(game.nextMap));
	STRNCPY(game.playerStart, playerStart, sizeof(game.playerStart));
}

void setNextLevelFromScript(char *token)
{
	char *name, *playerStart;
	
	name = strtok(token, " ");
	playerStart = strtok(NULL, "\0");
	
	setNextLevel(name, playerStart);
	
	setTransition(TRANSITION_OUT, &goToNextMap);
}

void goToNextMap()
{
	Entity *start;

	saveTemporaryData();

	printf("Freeing Resources\n");

	freeLevelResources();

	printf("Loading Map\n");

	if (hasPersistance(game.nextMap) == FALSE)
	{
		loadMap(game.nextMap, TRUE);
	}

	else
	{
		printf("Loading persistance data instead...\n");

		loadMap(game.nextMap, FALSE);

		loadPersitanceData(game.nextMap);
	}

	printf("Done\n");
	
	if (strcmpignorecase(game.playerStart, "PLAYER_START") != 0)
	{
		start = getEntityByObjectiveName(game.playerStart);
	
		if (start == NULL)
		{
			printf("Could not find player start %s\n", game.playerStart);
	
			exit(1);
		}
	
		loadPlayer(start->x, start->y, NULL);
	}

	game.nextMap[0] = '\0';
	game.playerStart[0] = '\0';

	clearScreen(0, 0, 0);
}

void setCheckpoint(float x, float y)
{
	game.checkpointX = x;
	game.checkpointY = y;
}

void getCheckpoint(float *x, float *y)
{
	*x = game.checkpointX;
	*y = game.checkpointY;
}

void pauseGame()
{
	game.paused = game.paused == TRUE ? FALSE : TRUE;

	if (game.pauseSurface == NULL)
	{
		game.pauseSurface = addBorder(generateTextSurface(_("Paused"), game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);
	}
}

void showPauseDialog()
{
	drawImage(game.pauseSurface, (game.screen->w - game.pauseSurface->w) / 2, (game.screen->h - game.pauseSurface->h) / 2, FALSE);
}
