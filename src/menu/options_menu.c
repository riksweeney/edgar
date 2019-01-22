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
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "cheat_menu.h"
#include "control_menu.h"
#include "label.h"
#include "main_menu.h"
#include "ok_menu.h"
#include "sound_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;

static Menu menu;
static char lastKeys[MAX_VALUE_LENGTH];
static int lastKeysIndex = 0;

static void loadMenuLayout(void);
static void toggleHints(void);
static void showControlMenu(void);
static void showSoundMenu(void);
static void showMainMenu(void);
static void showCheatMenu(void);
static void showCheatMenuWarn(void);
static void doMenu(void);
static void toggleFullscreen(void);
static void enableCheatMenu(void);
static void showOptionsMenu(void);

void drawOptionsMenu()
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
	int i, left, right, up, down, attack, xAxisMoved, yAxisMoved;
	char c;
	Widget *w;

	if (game.cheatsEnabled == FALSE && input.lastPressedKey >= 'A' && input.lastPressedKey <= 'z')
	{
		c = tolower(input.lastPressedKey);
		
		lastKeys[lastKeysIndex] = c;

		lastKeysIndex++;

		lastKeys[lastKeysIndex] = '\0';

		if (lastKeysIndex + 1 >= MAX_VALUE_LENGTH)
		{
			for (i=1;i<MAX_VALUE_LENGTH;i++)
			{
				lastKeys[i - 1] = lastKeys[i];

				lastKeys[i] = '\0';
			}

			lastKeysIndex = MAX_VALUE_LENGTH - 2;
		}

		if (strstr(lastKeys, MENU_CODE) != NULL)
		{
			enableCheatMenu();

			playSound("sound/common/faster");
		}
	}

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

		while (menu.widgets[menu.index]->hidden == TRUE);

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

		while (menu.widgets[menu.index]->hidden == TRUE);

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

	input.lastPressedKey = -1;

	input.xAxisMoved = xAxisMoved;
	input.yAxisMoved = yAxisMoved;
}

static void loadMenuLayout()
{
	int x, y, w, i, maxWidth;

	menu.widgetCount = 6;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Options Menu");
	}

	y = x = 0;

	menu.widgets[0] = createWidget(_("Configure Controls"), NULL, NULL, NULL, &showControlMenu, -1, y, TRUE, 255, 255, 255);

	menu.widgets[1] = createWidget(_("Configure Sound"), NULL, NULL, NULL, &showSoundMenu, -1, y, TRUE, 255, 255, 255);

	menu.widgets[2] = createWidget(_("Fullscreen"), NULL, &toggleFullscreen, &toggleFullscreen, &toggleFullscreen, x, y, TRUE, 255, 255, 255);

	menu.widgets[2]->label = createLabel(game.fullscreen == TRUE ? _("Yes") : _("No"), menu.widgets[2]->x + menu.widgets[2]->normalState->w + 10, y);

	menu.widgets[3] = createWidget(_("Show Hints"), NULL, &toggleHints, &toggleHints, &toggleHints, x, y, TRUE, 255, 255, 255);

	menu.widgets[3]->label = createLabel(game.showHints == TRUE ? _("Yes") : _("No"), menu.widgets[3]->x + menu.widgets[3]->normalState->w + 10, y);

	menu.widgets[4] = createWidget(_("Cheats"), NULL, NULL, NULL, &showCheatMenuWarn, -1, y, TRUE, 255, 255, 255);

	menu.widgets[4]->hidden = game.cheatsEnabled == TRUE ? FALSE : TRUE;

	menu.widgets[5] = createWidget(_("Back"), NULL, NULL, NULL, &showMainMenu, -1, y, TRUE, 255, 255, 255);

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

		y += menu.widgets[i]->selectedState->h + BUTTON_PADDING;
	}

	menu.w = w + BUTTON_PADDING;
	menu.h = y - BORDER_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initOptionsMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	input.lastPressedKey = -1;

	lastKeysIndex = 0;

	lastKeys[lastKeysIndex] = '\0';

	menu.returnAction = &showMainMenu;

	return &menu;
}

void freeOptionsMenu()
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

static void toggleHints()
{
	Widget *w = menu.widgets[menu.index];

	game.showHints = game.showHints == TRUE ? FALSE : TRUE;

	updateLabelText(w->label, game.showHints == TRUE ? _("Yes") : _("No"));
}

static void toggleFullscreen()
{
	Widget *w = menu.widgets[menu.index];

	game.fullscreen = game.fullscreen == TRUE ? FALSE : TRUE;

	toggleFullScreen();

	updateLabelText(w->label, game.fullscreen == TRUE ? _("Yes") : _("No"));
}

static void showControlMenu()
{
	game.menu = initControlMenu();

	game.drawMenu = &drawControlMenu;
}

static void showSoundMenu()
{
	game.menu = initSoundMenu();

	game.drawMenu = &drawSoundMenu;
}

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}

static void showCheatMenuWarn()
{
	if (game.previousStatus == IN_TITLE)
	{
		game.menu = initOKMenu(_("Cheats can only be enabled in-game"), &showOptionsMenu);
	}

	else
	{
		game.menu = initOKMenu(_("Enabling cheats will not allow you to achieve 100% completion in the game"), &showCheatMenu);
	}

	game.drawMenu = &drawOKMenu;
}

static void showCheatMenu()
{
	game.menu = initCheatMenu();

	game.drawMenu = &drawCheatMenu;
}

static void enableCheatMenu()
{
	int i;

	for (i=0;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->hidden == TRUE)
		{
			menu.widgets[i]->hidden = FALSE;
		}
	}

	game.cheatsEnabled = TRUE;
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}
