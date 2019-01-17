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
#include "draw.h"
#include "game.h"
#include "graphics/graphics.h"
#include "map.h"
#include "system/error.h"
#include "system/random.h"
#include "weather.h"

static Droplet droplet[MAX_DROPS];
extern Game game;
extern Entity player;

static void initLightRain(void);
static void initHeavyRain(void);
static void initStorm(void);
static void initSnow(void);
static void rain(void);
static void snow(void);
static void storm(void);
static void drawRain(void);
static void drawSnow(void);

static Type weatherType[] = {
					{NO_WEATHER, "NO_WEATHER"},
					{LIGHT_RAIN, "LIGHT_RAIN"},
					{HEAVY_RAIN, "HEAVY_RAIN"},
					{STORMY, "STORMY"},
					{SNOW, "SNOW"},
					};
static int weatherLength = sizeof(weatherType) / sizeof(Type);

void setWeather(int weatherType)
{
	switch (weatherType)
	{
		case LIGHT_RAIN:
			game.weatherAction = &initLightRain;
			game.weatherDraw = &drawRain;
		break;

		case HEAVY_RAIN:
			game.weatherAction = &initHeavyRain;
			game.weatherDraw = &drawRain;
		break;

		case STORMY:
			game.weatherAction = &initStorm;
			game.weatherDraw = &drawRain;
		break;

		case SNOW:
			game.weatherAction = &initSnow;
			game.weatherDraw = &drawSnow;
		break;

		default:
			game.weatherAction = NULL;
			game.weatherDraw = NULL;
		break;

	}

	game.weatherType = weatherType;
	game.weatherThinkTime = 60;
}

static void initLightRain()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS/4;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0;
		droplet[i].dirY = 8 + prand() % 8;

		droplet[i].active = TRUE;
	}

	game.weatherAction = &rain;
}

static void initHeavyRain()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS/2;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0;
		droplet[i].dirY = 8 + prand() % 8;

		droplet[i].active = TRUE;
	}

	game.weatherAction = &rain;
}

static void rain()
{
	int i;

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			droplet[i].y += droplet[i].dirY;

			if (droplet[i].y >= SCREEN_HEIGHT)
			{
				droplet[i].y = -8 - prand() % 20;

				droplet[i].dirX = 0;
				droplet[i].dirY = 8 + prand() % 8;
			}
		}

		else
		{
			break;
		}
	}
}

static void initStorm()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS/2;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0;
		droplet[i].dirY = 8 + prand() % 8;

		droplet[i].active = TRUE;
	}

	game.weatherAction = &storm;
}

static void storm()
{
	rain();

	game.weatherThinkTime--;
}

static void initSnow()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
		droplet[i].dirY = 0.5f + 0.1f * (prand() % 6);

		droplet[i].active = TRUE;
	}

	game.weatherAction = &snow;
}

static void snow()
{
	int i;

	for (i=0;i<MAX_DROPS;i++)
	{
		droplet[i].x += droplet[i].dirX;
		droplet[i].y += droplet[i].dirY;

		if (droplet[i].x < 0)
		{
			droplet[i].x = SCREEN_WIDTH + droplet[i].x;
		}

		else if (droplet[i].x > SCREEN_WIDTH)
		{
			droplet[i].x = droplet[i].x - SCREEN_WIDTH;
		}

		if (prand() % 30 == 0)
		{
			droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
		}

		if (droplet[i].y >= SCREEN_HEIGHT)
		{
			droplet[i].x = prand() % SCREEN_WIDTH;
			droplet[i].y = 0;

			droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
			droplet[i].dirY = 0.5f + 0.1f * (prand() % 6);
		}
	}
}

static void drawRain()
{
	int i;

	if (game.weatherThinkTime <= 0)
	{
		i = playSoundToMap("sound/enemy/thunder_cloud/lightning", -1, player.x, player.y, 0);

		fadeFromColour(255, 255, 255, 30);

		game.weatherThinkTime = 600 + i * 60;
	}

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			drawBox(droplet[i].x, droplet[i].y, 1, 8, 220, 220, 220, 255);
		}
	}
}

static void drawSnow()
{
	int i;

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			drawBox(droplet[i].x, droplet[i].y, 2, 2, 255, 255, 255, 255);
		}
	}
}

int getWeatherTypeByName(char *name)
{
	int i;

	for (i=0;i<weatherLength;i++)
	{
		if (strcmpignorecase(name, weatherType[i].name) == 0)
		{
			return weatherType[i].id;
		}
	}

	showErrorAndExit("Unknown Weather Type %s", name);

	return 0;
}

char *getWeatherTypeByID(int id)
{
	int i;

	for (i=0;i<weatherLength;i++)
	{
		if (id == weatherType[i].id)
		{
			return weatherType[i].name;
		}
	}

	showErrorAndExit("Unknown Weather ID %d", id);

	return 0;
}

char *getWeather()
{
	return getWeatherTypeByID(game.weatherType);
}
