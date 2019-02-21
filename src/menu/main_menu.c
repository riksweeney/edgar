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

#include "../headers.h"

#include "../audio/audio.h"
#include "../audio/music.h"
#include "../game.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/load_save.h"
#include "about_menu.h"
#include "io_menu.h"
#include "medals_menu.h"
#include "ok_menu.h"
#include "options_menu.h"
#include "stats_menu.h"
#include "widget.h"
#include "yes_no_menu.h"

extern Input input, menuInput;
extern Game game;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static void showIOMenu(void);
static void doNewGame(void);
static void doTutorial(void);
static void doQuit(void);
static void showMainMenu(void);
static void showAboutMenu(void);
static void showStatsMenu(void);
static void showMedalsMenu(void);
static void quitToTitle(void);
static void continueGame(void);
static void restartCheckpoint(void);

void drawMainMenu()
{
	int i;

	drawImage(menu.background, menu.x, menu.y, FALSE, 196);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], &menu, menu.index == i);
	}
}

static void doMenu()
{
	Widget *w;
	int left, right, up, down, attack, xAxisMoved, yAxisMoved;

	left = FALSE;
	right = FALSE;
	up = FALSE;
	down = FALSE;
	attack = FALSE;

	if (menuInput.left == TRUE)
	{
		left = TRUE;
	}

	else if (menuInput.right == TRUE)
	{
		right = TRUE;
	}

	else if (menuInput.up == TRUE)
	{
		up = TRUE;
	}

	else if (menuInput.down == TRUE)
	{
		down = TRUE;
	}

	else if (menuInput.attack == TRUE)
	{
		attack = TRUE;
	}

	else if (input.left == TRUE)
	{
		left = TRUE;
	}

	else if (input.right == TRUE)
	{
		right = TRUE;
	}

	else if (input.up == TRUE)
	{
		up = TRUE;
	}

	else if (input.down == TRUE)
	{
		down = TRUE;
	}

	else if (input.attack == TRUE)
	{
		attack = TRUE;
	}

	if (down == TRUE)
	{
		do
		{
			menu.index++;

			if (menu.index >= menu.widgetCount)
			{
				menu.index = 0;
			}
		}

		while (menu.widgets[menu.index]->disabled == TRUE || menu.widgets[menu.index]->hidden == TRUE);

		playSound("sound/common/click");
	}

	else if (up == TRUE)
	{
		do
		{
			menu.index--;

			if (menu.index < 0)
			{
				menu.index = menu.widgetCount - 1;
			}
		}

		while (menu.widgets[menu.index]->disabled == TRUE || menu.widgets[menu.index]->hidden == TRUE);

		playSound("sound/common/click");
	}

	else if (attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->clickAction != NULL)
		{
			playSound("sound/common/click");

			w->clickAction();
		}
	}

	else if (left == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->rightAction != NULL)
		{
			playSound("sound/common/click");

			w->rightAction();
		}
	}

	else if (right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			playSound("sound/common/click");

			w->leftAction();
		}
	}

	xAxisMoved = input.xAxisMoved;
	yAxisMoved = input.yAxisMoved;

	memset(&menuInput, 0, sizeof(Input));
	memset(&input, 0, sizeof(Input));

	input.xAxisMoved = xAxisMoved;
	input.yAxisMoved = yAxisMoved;
}

static void loadMenuLayout()
{
	int i, x, y, w, maxWidth;

	y = x = -1;

	menu.widgetCount = 10;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Main Menu");
	}

	menu.widgets[0] = createWidget(_("New Game"), NULL, NULL, NULL, &doNewGame, x, y, TRUE, 255, 255, 255);

	menu.widgets[1] = createWidget(_("Continue"), NULL, NULL, NULL, &continueGame, x, y, TRUE, 255, 255, 255);

	menu.widgets[1]->disabled = game.canContinue == TRUE ? FALSE : TRUE;

	menu.widgets[2] = createWidget(_("Restart Checkpoint"), NULL, NULL, NULL, &restartCheckpoint, x, y, TRUE, 255, 255, 255);

	menu.widgets[2]->disabled = game.canContinue == TRUE ? FALSE : TRUE;

	menu.widgets[3] = createWidget(_("Tutorial"), NULL, NULL, NULL, &doTutorial, x, y, TRUE, 255, 255, 255);

	menu.widgets[4] = createWidget(_("Load Game"), NULL, NULL, NULL, &showIOMenu, x, y, TRUE, 255, 255, 255);

	menu.widgets[5] = createWidget(_("Options"), NULL, NULL, NULL, &showOptionsMenu, x, y, TRUE, 255, 255, 255);

	menu.widgets[6] = createWidget(_("Statistics"), NULL, NULL, NULL, &showStatsMenu, x, y, TRUE, 255, 255, 255);

	menu.widgets[7] = createWidget(_("Medals"), NULL, NULL, NULL, &showMedalsMenu, x, y, TRUE, 255, 255, 255);

	menu.widgets[8] = createWidget(_("About"), NULL, NULL, NULL, &showAboutMenu, x, y, TRUE, 255, 255, 255);

	menu.widgets[9] = createWidget(_("Quit"), NULL, NULL, NULL, &doQuit, x, y, TRUE, 255, 255, 255);

	y = BUTTON_PADDING + BORDER_PADDING;

	w = 0;

	maxWidth = 0;

	for (i=0;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth)
		{
			maxWidth = menu.widgets[i]->normalState->w;
		}
	}

	for (i=0;i<menu.widgetCount;i++)
	{
		menu.widgets[i]->y = y;

		if (menu.widgets[i]->x != -1)
		{
			menu.widgets[i]->x = BUTTON_PADDING + BORDER_PADDING;
		}

		if (menu.widgets[i]->label != NULL)
		{
			menu.widgets[i]->label->y = y;

			menu.widgets[i]->label->x = menu.widgets[i]->x + maxWidth + 10;

			if (menu.widgets[i]->label->x + menu.widgets[i]->label->text->w > w)
			{
				w = menu.widgets[i]->label->x + menu.widgets[i]->label->text->w;
			}
		}

		else
		{
			if (menu.widgets[i]->x + menu.widgets[i]->selectedState->w > w)
			{
				w = menu.widgets[i]->x + menu.widgets[i]->selectedState->w;
			}
		}

		if (i != 1)
		{
			y += menu.widgets[i]->selectedState->h + BUTTON_PADDING;
		}
	}

	menu.w = w + BUTTON_PADDING;
	menu.h = y - BORDER_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

void freeMainMenu()
{
	int i;

	if (menu.widgets != NULL)
	{
		for (i=0;i<menu.widgetCount;i++)
		{
			freeWidget(menu.widgets[i]);
		}

		free(menu.widgets);

		menu.widgets = NULL;
	}

	if (menu.background != NULL)
	{
		destroyTexture(menu.background);

		menu.background = NULL;
	}
}

Menu *initMainMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	menu.widgets[1]->disabled = game.canContinue == TRUE ? FALSE : TRUE;

	menu.widgets[1]->hidden = game.previousStatus == IN_TITLE ? FALSE : TRUE;

	menu.widgets[2]->disabled = game.canContinue == TRUE ? FALSE : TRUE;

	menu.widgets[2]->hidden = game.previousStatus == IN_GAME ? FALSE : TRUE;

	menu.widgets[6]->disabled = game.previousStatus == IN_GAME ? FALSE : TRUE;

	menu.returnAction = NULL;

	return &menu;
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}

static void showAboutMenu()
{
	game.menu = initAboutMenu();

	game.drawMenu = &drawAboutMenu;
}

static void showStatsMenu()
{
	game.menu = initStatsMenu();

	game.drawMenu = &drawStatsMenu;
}

static void showIOMenu()
{
	game.menu = initIOMenu(FALSE);

	game.drawMenu = &drawIOMenu;
}

static void showMedalsMenu()
{
	game.menu = initMedalsMenu();

	game.drawMenu = &drawMedalsMenu;
}

static void doNewGame()
{
	if (game.previousStatus == IN_TITLE)
	{
		newGame();
	}

	else
	{
		game.menu = initYesNoMenu(_("Start a new game?"), &newGame, &showMainMenu);

		game.drawMenu = &drawYesNoMenu;
	}
}

static void doTutorial()
{
	if (game.previousStatus == IN_TITLE)
	{
		tutorial();
	}

	else
	{
		game.menu = initYesNoMenu(_("Play the tutorial?"), &tutorial, &showMainMenu);

		game.drawMenu = &drawYesNoMenu;
	}
}

static void doQuit()
{
	if (game.previousStatus == IN_TITLE)
	{
		quitGame();
	}

	else
	{
		game.menu = initYesNoMenu(_("Exit the game?"), &quitToTitle, &showMainMenu);

		game.drawMenu = &drawYesNoMenu;
	}
}

static void continueGame()
{
	if (loadGame(getMostRecentSave()) == TRUE)
	{
		menu.index = 0;

		menu.returnAction = NULL;

		freeMessageQueue();

		setInfoBoxMessage(60, 255, 255, 255, _("Game Loaded"));

		game.paused = FALSE;
	}

	else
	{
		game.menu = initOKMenu(_("Failed to load game"), &showIOMenu);

		game.drawMenu = &drawOKMenu;
	}
}

static void restartCheckpoint()
{
	getContinuePoint();
}

static void quitToTitle()
{
	menu.index = 0;

	pauseGame();

	setTransition(TRANSITION_OUT, &titleScreen);

	fadeOutMusic(500);
}

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}
