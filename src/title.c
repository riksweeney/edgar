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

#include "audio/music.h"
#include "game.h"
#include "graphics/animation.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "player.h"
#include "system/load_save.h"
#include "system/random.h"

extern Input menuInput, input;
extern Game game;
extern Entity player;

static Title title;

static void initTitle(void);

void doTitle()
{
	if (title.edgarLogo == NULL)
	{
		initTitle();
	}

	title.thinkTime--;

	if (title.thinkTime <= -30)
	{
		title.thinkTime = 30;
	}
}

void drawTitle()
{
	if (title.edgarLogo != NULL)
	{
		drawImage(title.edgarLogo, (SCREEN_WIDTH - title.edgarLogo->w) / 2, SCREEN_HEIGHT / 8, FALSE, 255);

		drawImage(title.copyright, (SCREEN_WIDTH - title.copyright->w) / 2, SCREEN_HEIGHT - title.copyright->h - 5, FALSE, 255);

		drawLoopingAnimation(&player, (SCREEN_WIDTH - player.w) / 2, SCREEN_HEIGHT / 2 + player.h / 2, 0, 0, 0);

		if (title.thinkTime <= 0)
		{
			drawImage(title.startButton, (SCREEN_WIDTH - title.startButton->w) / 2, SCREEN_HEIGHT * 6 / 8, FALSE, 255);
		}
	}

	drawGame();
}

static void initTitle()
{
	char copyright[MAX_VALUE_LENGTH];
	SDL_Surface *surface;

	snprintf(copyright, MAX_VALUE_LENGTH, _("Copyright Parallel Realities 2009 - %d"), YEAR);

	surface = generateTransparentTextSurface(copyright, game.font, 220, 220, 220, TRUE);
	
	title.copyright = convertSurfaceToTexture(surface, TRUE);

	title.edgarLogo = loadImage("gfx/title_screen/logo.png");

	surface = generateTransparentTextSurface(_("Press any key"), game.largeFont, 220, 220, 220, TRUE);
	
	title.startButton = convertSurfaceToTexture(surface, TRUE);

	title.thinkTime = 30;

	loadPlayer(0, 0, "edgar/edgar_title");

	setEntityAnimationByID(&player, prand() % 3);

	title.continueSlot = getMostRecentSave();

	if (title.continueSlot != -1)
	{
		game.canContinue = TRUE;
	}

	setTransition(TRANSITION_IN, NULL);

	loadMusic("TITLE_MUSIC");

	playLoadedMusic();
}

void freeTitle()
{
	if (title.edgarLogo != NULL)
	{
		destroyTexture(title.edgarLogo);

		title.edgarLogo = NULL;
	}

	if (title.copyright != NULL)
	{
		destroyTexture(title.copyright);

		title.copyright = NULL;
	}

	if (title.startButton != NULL)
	{
		destroyTexture(title.startButton);

		title.startButton = NULL;
	}
}
