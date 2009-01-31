#include "headers.h"

#include "game.h"
#include "weather.h"
#include "font.h"

extern Game game;

void initGame()
{
	game.weatherType = 0;

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
