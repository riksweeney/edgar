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

#include "headers.h"

#include "audio/audio.h"
#include "audio/music.h"
#include "entity.h"
#include "event/map_trigger.h"
#include "game.h"
#include "graphics/graphics.h"
#include "hud.h"
#include "inventory.h"
#include "map.h"
#include "medal.h"
#include "menu/io_menu.h"
#include "menu/main_menu.h"
#include "menu/menu.h"
#include "player.h"
#include "system/error.h"
#include "system/load_save.h"
#include "system/random.h"
#include "system/resources.h"

extern Game game;
extern Entity *self;

static ContinueData continueData;

static void shake(void);
static void wipeOutRightToLeft(void);
static void wipeOutLeftToRight(void);
static void wipeInRightToLeft(void);
static void wipeInLeftToRight(void);
static void wipeInCircleToLarge(void);
static void wipeInCircleToSmall(void);
static void wipeOutCircleToLarge(void);
static void wipeOutCircleToSmall(void);
static void creditsWipeOut(void);
static void fadeToNormal(void);
static void initEndCredits(void);

void initGame()
{
	game.drawScreen = TRUE;

	if (game.paused == TRUE)
	{
		pauseGame();
	}

	game.status = IN_GAME;

	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;

	game.shakeThinkTime = 0;

	game.kills = 0;

	game.batsDrowned = 0;

	game.timesEaten = 0;

	game.arrowsFired = 0;

	game.playTime = 0;

	game.distanceTravelled = 0;

	game.timeSpentAsSlime = 0;

	game.attacksBlocked = 0;

	game.thinkTime = 0;

	game.continues = 0;

	game.secretsFound = 0;

	game.cheating = FALSE;

	game.infiniteArrows = FALSE;

	game.infiniteEnergy = FALSE;

	game.lavaIsFatal = TRUE;

	game.mapExitable = 0;

	game.canContinue = FALSE;

	game.overrideMusic = FALSE;

	game.showHUD = TRUE;

	game.offsetX = game.offsetY = 0;
	
	if (game.pauseSurface != NULL)
	{
		destroyTexture(game.pauseSurface);

		game.pauseSurface = NULL;
	}

	if (game.gameOverSurface != NULL)
	{
		destroyTexture(game.gameOverSurface);

		game.gameOverSurface = NULL;
	}
}

void doGame()
{
	/* Execute the action if there is one */

	doGameOver();

	fadeToNormal();

	if (game.shakeThinkTime > 0 || game.shakeThinkTime == -1)
	{
		shake();

		if (game.shakeThinkTime > 0)
		{
			game.shakeThinkTime--;

			if (game.shakeThinkTime <= 0)
			{
				game.offsetX = game.offsetY = 0;
			}
		}
	}

	if (game.weatherType != NO_WEATHER)
	{
		game.weatherAction();
	}

	game.playTime++;
}

void freeGame()
{
	if (game.pauseSurface != NULL)
	{
		destroyTexture(game.pauseSurface);

		game.pauseSurface = NULL;
	}
	
	if (game.gameOverSurface != NULL)
	{
		destroyTexture(game.gameOverSurface);

		game.gameOverSurface = NULL;
	}
}

void drawWeather()
{
	if (game.weatherType != NO_WEATHER)
	{
		game.weatherDraw();
	}
}

void drawGame()
{
	float alpha;
	
	if (game.alphaSurface.w != 0 && game.alphaSurface.h != 0)
	{
		alpha = 255.0f / game.alphaTime;

		alpha *= game.thinkTime;

		drawBox(game.alphaSurface.x, game.alphaSurface.y, game.alphaSurface.w, game.alphaSurface.h, game.alphaSurface.r, game.alphaSurface.g, game.alphaSurface.b, alpha);
	}

	drawGameOver();

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
	game.offsetX = game.offsetY = 0;

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
	if (game.transitionX <= -15)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			clearScreen(0, 0, 0);

			game.transitionCallback();
		}

		return;
	}

	drawBox(game.transitionX < 0 ? 0 : game.transitionX, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);

	game.transitionX -= 15;
}

static void wipeOutLeftToRight()
{
	if (game.transitionX > SCREEN_WIDTH + 15)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			clearScreen(0, 0, 0);

			game.transitionCallback();
		}

		return;
	}

	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0, 255);

	game.transitionX += 15;
}

static void wipeInLeftToRight()
{
	if (game.transitionX >= SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			clearScreen(0, 0, 0);

			game.transitionCallback();
		}

		return;
	}

	drawBox(game.transitionX, 0, SCREEN_WIDTH - game.transitionX, SCREEN_HEIGHT, 0, 0, 0, 255);

	game.transitionX += 15;
}

static void wipeInRightToLeft()
{
	if (game.transitionX < 0)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			clearScreen(0, 0, 0);

			game.transitionCallback();
		}

		return;
	}

	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0, 255);

	game.transitionX -= 15;
}

static void wipeInCircleToLarge()
{
	if (game.transitionX > SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			clearScreen(0, 0, 0);

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
			clearScreen(0, 0, 0);

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
			clearScreen(0, 0, 0);

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
			clearScreen(0, 0, 0);

			game.transitionCallback();
		}

		return;
	}

	drawCircleFromSurface(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX < 0 ? 0 : game.transitionX);

	game.transitionX -= 10;
}

static void creditsWipeOut()
{
	if (game.transitionX <= -5)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			clearScreen(0, 0, 0);

			game.transitionCallback();
		}

		return;
	}

	drawCircleFromSurface(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX < 0 ? 0 : game.transitionX);

	game.transitionX -= 5;

	if (game.transitionX <= spotlightSize() / 2)
	{
		if (game.transitionY > 0)
		{
			game.transitionX = spotlightSize() / 2;

			game.transitionY--;
		}
	}
}

void setNextLevel(char *name, char *playerStart)
{
	STRNCPY(game.nextMap, name, sizeof(game.nextMap));
	STRNCPY(game.playerStart, playerStart, sizeof(game.playerStart));
}

void setNextLevelFromScript(char *token)
{
	char *name, *playerStart, *savePtr;

	name = strtok_r(token, " ", &savePtr);
	playerStart = strtok_r(NULL, "\0", &savePtr);

	setNextLevel(name, playerStart);

	setTransition(TRANSITION_OUT, &goToNextMap);

	if (game.overrideMusic == FALSE)
	{
		fadeOutMusic(1000);
	}
}

void showEndCredits()
{
	game.overrideMusic = FALSE;

	game.transitionX = SCREEN_WIDTH;
	game.transitionY = 120;
	game.transition = &creditsWipeOut;

	game.transitionCallback = &initEndCredits;
}

static void initEndCredits()
{
	game.status = IN_CREDITS;
}

void goToNextMap()
{
	Entity *start;

	saveTemporaryData();

	freeLevelResources();

	if (hasPersistance(game.nextMap) == FALSE)
	{
		loadMap(game.nextMap, TRUE);
	}

	else
	{
		loadMap(game.nextMap, FALSE);

		loadPersitanceData(game.nextMap);
	}

	if (strcmpignorecase(game.playerStart, "PLAYER_START") != 0)
	{
		start = getEntityByObjectiveName(game.playerStart);

		if (start == NULL)
		{
			showErrorAndExit("Could not find player start %s", game.playerStart);
		}

		loadPlayer(start->startX, start->startY, NULL);
	}

	fireMapTrigger(game.nextMap);

	game.nextMap[0] = '\0';
	game.playerStart[0] = '\0';

	game.offsetX = game.offsetY = 0;
	game.shakeThinkTime = 0;

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
	switch (game.status)
	{
		case IN_GAME:
		case IN_TITLE:
			game.paused = TRUE;

			game.previousStatus = game.status;

			game.status = IN_MENU;

			game.menu = initMainMenu();
			
			if (game.pauseSurface == NULL)
			{
				game.pauseSurface = copyScreen();
			}

			if (game.previousStatus == IN_GAME)
			{
				pauseMusic(TRUE);

				pauseSound(TRUE);
			}
		break;

		case IN_EDITOR:
			exit(0);
		break;

		case IN_INVENTORY:
		case IN_CREDITS:
			/* Do nothing */
		break;

		default:
			if (game.menu->returnAction == NULL)
			{
				if (game.pauseSurface != NULL)
				{
					destroyTexture(game.pauseSurface);

					game.pauseSurface = NULL;
				}
				
				game.paused = FALSE;

				game.status = game.previousStatus;

				game.menu = initMainMenu();

				game.drawMenu = &drawMainMenu;

				pauseMusic(FALSE);

				pauseSound(FALSE);
			}
		break;
	}
}

void showSaveDialog()
{
	pauseGame();

	pauseMusic(FALSE);

	pauseSound(FALSE);

	game.status = IN_MENU;

	game.paused = TRUE;

	game.menu = initIOMenu(TRUE);

	game.drawMenu = &drawIOMenu;
}

void pauseGameInventory()
{
	switch (game.status)
	{
		case IN_GAME:
			game.status = IN_INVENTORY;

			clearInventoryDescription();
			
			if (game.pauseSurface == NULL)
			{
				game.pauseSurface = copyScreen();
			}
		break;

		case IN_INVENTORY:
			if (game.pauseSurface != NULL)
			{
				destroyTexture(game.pauseSurface);

				game.pauseSurface = NULL;
			}
			
			game.status = IN_GAME;
		break;
	}
}

void focusLost()
{
	#if DEV == 0
		if (game.paused == FALSE && game.status == IN_GAME)
		{
			pauseGame();
		}
	#endif
}

void showPauseDialog()
{
	drawImage(game.pauseSurface, 0, 0, FALSE, 255);
}

void resetGameSettings()
{
	game.audio = TRUE;
	game.sfxDefaultVolume = 10;
	game.musicDefaultVolume = 8;
	game.showHints = TRUE;
	game.fullscreen = FALSE;
	game.audioQuality = 22050;
}

void writeGameSettingsToFile(FILE *fp)
{
	fprintf(fp, "GAME_SETTINGS\n");
	fprintf(fp, "AUDIO %d\n", game.audio == TRUE ? TRUE : game.audioDisabled);
	fprintf(fp, "SFX_VOLUME %d\n", game.sfxDefaultVolume);
	fprintf(fp, "MUSIC_VOLUME %d\n", game.musicDefaultVolume);
	fprintf(fp, "HINTS %d\n", game.showHints);
	fprintf(fp, "FULLSCREEN %d\n", game.fullscreen);
	fprintf(fp, "AUDIO_QUALITY %d\n", game.audioQuality);
	fprintf(fp, "FONT %s\n", game.customFont);
	fprintf(fp, "SMALL_FONT_SIZE %d\n", game.fontSizeSmall == 0 ? NORMAL_FONT_SIZE : game.fontSizeSmall);
	fprintf(fp, "LARGE_FONT_SIZE %d\n", game.fontSizeLarge == 0 ? LARGE_FONT_SIZE : game.fontSizeLarge);
}

void readGameSettingsFromFile(char *buffer)
{
	char *line, *token, *savePtr;

	savePtr = NULL;

	line = strtok_r(buffer, "\n", &savePtr);

	while (line != NULL)
	{
		token = strtok(line, " ");

		if (strcmpignorecase(token, "AUDIO") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.audio = atoi(token);
			}
		}

		else if (strcmpignorecase(token, "SFX_VOLUME") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.sfxDefaultVolume = atoi(token);
			}
		}

		else if (strcmpignorecase(token, "MUSIC_VOLUME") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.musicDefaultVolume = atoi(token);
			}
		}

		else if (strcmpignorecase(token, "HINTS") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.showHints = atoi(token);
			}
		}

		else if (strcmpignorecase(token, "FULLSCREEN") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.fullscreen = atoi(token);
			}
		}

		else if (strcmpignorecase(token, "AUDIO_QUALITY") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.audioQuality = atoi(token);
			}

			if (game.audioQuality == 0)
			{
				game.audioQuality = 22050;
			}
		}

		else if (strcmpignorecase(token, "FONT") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				STRNCPY(game.customFont, token, MAX_FILE_LENGTH);
			}
		}

		else if (strcmpignorecase(token, "SMALL_FONT_SIZE") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.fontSizeSmall = atoi(token);
			}
		}

		else if (strcmpignorecase(token, "LARGE_FONT_SIZE") == 0)
		{
			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				game.fontSizeLarge = atoi(token);
			}
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	if (strlen(game.customFont) != 0)
	{
		if (game.fontSizeSmall <= 0 || game.fontSizeLarge <= 0)
		{
			showErrorAndExit(_("SMALL_FONT_SIZE and LARGE_FONT_SIZE must be specified when using a custom font"));
		}
	}
}

void initGameOver()
{
	if (game.gameOverSurface == NULL)
	{
		game.gameOverSurface = loadImage("gfx/common/gameover.png");
	}

	game.gameOverX = 0;
}

void doGameOver()
{
	if (game.gameOverSurface != NULL)
	{
		game.gameOverX += 3;
	}
}

void drawGameOver()
{
	if (game.gameOverSurface != NULL)
	{
		if (game.gameOverX >= game.gameOverSurface->w)
		{
			game.gameOverX = game.gameOverSurface->w;
		}

		drawClippedImage(game.gameOverSurface, 0, 0, (SCREEN_WIDTH - game.gameOverSurface->w) / 2, (SCREEN_HEIGHT - game.gameOverSurface->h) / 2, game.gameOverX, game.gameOverSurface->h);
	}
}

char *getPlayTimeAsString()
{
	/* 1 second is 60 frames */

	int hours, minutes;
	long tempTime;
	char *timeString;

	timeString = malloc(15 * sizeof(char));

	if (timeString == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Play Time string...", 15 * (int)sizeof(char));
	}

	tempTime = game.playTime;

	hours = tempTime / (60 * 60 * 60);

	tempTime -= hours * 60 * 60 * 60;

	minutes = tempTime / (60 * 60);

	snprintf(timeString, 15, "%dH %dM", hours, minutes);

	return timeString;
}

char *getPlayTimeHours()
{
	/* 1 second is 60 frames */

	int hours;
	long tempTime;
	char *timeString;

	timeString = malloc(5 * sizeof(char));

	if (timeString == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Play Time Hours string...", 5 * (int)sizeof(char));
	}

	tempTime = game.playTime;

	hours = tempTime / (60 * 60 * 60);

	snprintf(timeString, 5, "%d", hours);

	return timeString;
}

char *getSlimeTimeAsString()
{
	/* 1 second is 60 frames */

	int minutes;
	long tempTime;
	char *timeString;

	timeString = malloc(15 * sizeof(char));

	if (timeString == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Slime Time string...", 15 * (int)sizeof(char));
	}

	tempTime = game.timeSpentAsSlime;

	minutes = tempTime / (60 * 60);

	tempTime -= minutes * 60 * 60;

	tempTime /= 60;

	snprintf(timeString, 15, "%dM %ldS", minutes, tempTime);

	return timeString;
}

void fadeFromColour(int r, int g, int b, int fadeTime)
{
	game.alphaTime = game.thinkTime = fadeTime;

	game.alphaSurface.x = 0;
	game.alphaSurface.y = 0;
	game.alphaSurface.w = SCREEN_WIDTH;
	game.alphaSurface.h = SCREEN_HEIGHT;
	
	game.alphaSurface.r = r;
	game.alphaSurface.g = g;
	game.alphaSurface.b = b;
	
	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0, 255);
}

static void fadeToNormal()
{
	if (game.alphaSurface.w != 0 && game.alphaSurface.h != 0)
	{
		game.thinkTime--;

		if (game.thinkTime <= 0)
		{
			game.alphaSurface.w = 0;
			game.alphaSurface.h = 0;
		}
	}
}

void increaseKillCount()
{
	game.kills++;

	if (game.kills == 100)
	{
		addMedal("kill_100");
	}

	else if (game.kills == 200)
	{
		addMedal("kill_200");
	}

	else if (game.kills == 500)
	{
		addMedal("kill_500");
	}

	else if (game.kills == 1000)
	{
		addMedal("kill_1000");
	}

	else if (game.kills == 2000)
	{
		addMedal("kill_2000");
	}
}

void increaseSecretsFound()
{
	game.secretsFound++;

	setInfoBoxMessage(90, 0, 255, 0, _("You found a secret!"));

	playSound("sound/common/secret");

	if (game.secretsFound == TOTAL_SECRETS)
	{
		addMedal("all_secrets");
	}
}

void setContinuePoint(int cameraFollow, char *boss, void (*resumeAction)(void))
{
	Entity *temp;

	temp = self;

	game.canContinue = TRUE;

	continueData.cameraMinX = getCameraMinX();
	continueData.cameraMinY = getCameraMinY();
	continueData.cameraMaxX = getCameraMaxX();
	continueData.cameraMaxY = getCameraMaxY();

	continueData.cameraFollow = cameraFollow;

	continueData.resumeAction = resumeAction;

	STRNCPY(continueData.boss, boss, sizeof(continueData.boss));

	saveContinueData();

	self = temp;
}

void getContinuePoint()
{
	Entity *e;

	loadContinueData();

	limitCamera(continueData.cameraMinX, continueData.cameraMinY, continueData.cameraMaxX, continueData.cameraMaxY);

	setMapStartX(continueData.cameraMinX);
	setMapStartY(continueData.cameraMinY);

	setCameraPosition(continueData.cameraMinX, continueData.cameraMinY);

	limitPlayerToCameraLimits();

	if (continueData.cameraFollow == FALSE)
	{
		centerMapOnEntity(NULL);
	}

	else
	{
		cameraSnapToTargetEntity();
	}

	if (continueData.resumeAction != NULL)
	{
		e = getEntityByName(continueData.boss);

		if (e == NULL)
		{
			showErrorAndExit("Continue data could not find %s", continueData.boss);
		}

		e->action = continueData.resumeAction;
	}

	game.continues++;

	if (game.continues == 20)
	{
		addMedal("continue_20");
	}
}

void clearContinuePoint()
{
	game.canContinue = FALSE;

	continueData.resumeAction = NULL;

	continueData.boss[0] = '\0';
}

void updateExitCount(int exitCount)
{
	game.mapExitable += exitCount;

	if (game.mapExitable < 0)
	{
		game.mapExitable = 0;
	}
}
