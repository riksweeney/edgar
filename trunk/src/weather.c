#include "headers.h"

#include "system/random.h"
#include "map.h"
#include "graphics/graphics.h"
#include "draw.h"

static Droplet droplet[MAX_DROPS];
extern Game game;

static void initLightRain(void);
static void initHeavyRain(void);
static void initStorm(void);
static void initSnow(void);
static void rain(void);
static void snow(void);
static void storm(void);
static void drawRain(void);
static void drawSnow(void);

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
	}

	game.weatherThinkTime = 60;
}

static void initLightRain()
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

static void initHeavyRain()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS;i++)
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
			droplet[i].x += droplet[i].dirX;
			droplet[i].y += droplet[i].dirY;

			if (droplet[i].y >= SCREEN_HEIGHT || mapTileAt(droplet[i].x / TILE_SIZE, droplet[i].y / TILE_SIZE) != BLANK_TILE)
			{
				droplet[i].x = prand() % SCREEN_WIDTH;
				droplet[i].y = -8 - prand() % 20;

				droplet[i].dirX = 0;
				droplet[i].dirY = 8 + prand() % 8;
			}
		}
	}
}

static void initStorm()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS;i++)
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

	if (game.weatherThinkTime < -60)
	{
		game.weatherThinkTime = 600 + prand() % 1200;
	}
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
		droplet[i].dirY = 01.f + 0.1f * (prand() % 10);

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

		if (prand() % 30 == 0)
		{
			droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
		}

		if (droplet[i].y >= SCREEN_HEIGHT || mapTileAt(droplet[i].x / TILE_SIZE, droplet[i].y / TILE_SIZE) != BLANK_TILE)
		{
			droplet[i].x = prand() % SCREEN_WIDTH;
			droplet[i].y = -8 - prand() % 20;

			droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
			droplet[i].dirY = 01.f + 0.1f * (prand() % 10);
		}
	}
}

static void drawRain()
{
	int i;

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			drawBox(droplet[i].x, droplet[i].y, 1, 8, 255, 255, 255);
		}
	}

	if (game.weatherThinkTime >= -10 && game.weatherThinkTime < 0)
	{
		clearScreen(255, 255, 255);
	}
}

static void drawSnow()
{
	int i;

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			drawBox(droplet[i].x, droplet[i].y, 2, 2, 255, 255, 255);
		}
	}
}
