#include "headers.h"

#include "game.h"
#include "weather.h"
#include "font.h"
#include "random.h"
#include "music.h"

extern Game game;

static void shake(void);

void initGame()
{
	game.weatherType = 0;

	setWeather(game.weatherType);
	
	playMusic();
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

void drawGame()
{
	if (game.weatherType != NO_WEATHER)
	{
		game.weatherDraw();
	}
}

void freeGame()
{
	closeFont(game.font);
}

void setGameSword(char *name)
{
	strcpy(game.sword, name);
}

void setGameShield(char *name)
{
	strcpy(game.shield, name);
}

char *getGameSword()
{
	return game.sword;
}

char *getGameShield()
{
	return game.shield;
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
