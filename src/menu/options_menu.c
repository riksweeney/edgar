/*
Copyright (C) 2009-2011 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/pak.h"
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
	int i;
	Widget *w;

	if (game.cheatsEnabled == FALSE && input.lastPressedKey != -1 && isalnum(input.lastPressedKey))
	{
		lastKeys[lastKeysIndex] = tolower(input.lastPressedKey);

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

			playSound("sound/common/faster.ogg");
		}

		input.lastPressedKey = -1;
	}

	if (input.down == TRUE || menuInput.down == TRUE)
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

		menuInput.down = FALSE;
		input.down = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.up == TRUE || menuInput.up == TRUE)
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

		menuInput.up = FALSE;
		input.up = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->clickAction != NULL)
		{
			menuInput.attack = FALSE;
			input.attack = FALSE;

			playSound("sound/common/click.ogg");

			w->clickAction();
		}
	}

	else if (input.left == TRUE || menuInput.left == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->rightAction != NULL)
		{
			menuInput.left = FALSE;
			input.left = FALSE;

			playSound("sound/common/click.ogg");

			w->rightAction();
		}
	}

	else if (input.right == TRUE || menuInput.right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			menuInput.right = FALSE;
			input.right = FALSE;

			playSound("sound/common/click.ogg");

			w->leftAction();
		}
	}
}

static void loadMenuLayout()
{
	char *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i;

	savePtr1 = NULL;

	i = 0;

	buffer = loadFileFromPak("data/menu/options_menu.dat");

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	while (line != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr1);

			continue;
		}

		token = strtok_r(line, " ", &savePtr2);

		if (strcmpignorecase(token, "WIDTH") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.w = atoi(token);
		}

		else if (strcmpignorecase(token, "HEIGHT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.h = atoi(token);
		}

		else if (strcmpignorecase(token, "WIDGET_COUNT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.widgetCount = atoi(token);

			menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				showErrorAndExit("Ran out of memory when creating Options Menu");
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "MENU_CONTROLS") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showControlMenu, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_SOUND") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showSoundMenu, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_HINTS") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, &toggleHints, &toggleHints, &toggleHints, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->label = createLabel(game.showHints == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);
				}

				else if (strcmpignorecase(menuID, "MENU_FULLSCREEN") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, &toggleFullscreen, &toggleFullscreen, &toggleFullscreen, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->label = createLabel(game.fullscreen == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);
				}

				else if (strcmpignorecase(menuID, "MENU_CHEAT") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showCheatMenuWarn, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->hidden = game.cheatsEnabled == TRUE ? FALSE : TRUE;
				}

				else if (strcmpignorecase(menuID, "MENU_BACK") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showMainMenu, x, y, TRUE, 255, 255, 255);
				}

				else
				{
					showErrorAndExit("Unknown widget %s", menuID);
				}

				i++;
			}

			else
			{
				showErrorAndExit("Widget Count must be defined!");
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w <= 0 || menu.h <= 0)
	{
		showErrorAndExit("Menu dimensions must be greater than 0");
	}

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	free(buffer);

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
	}

	if (menu.background != NULL)
	{
		SDL_FreeSurface(menu.background);

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
	game.menu = initOKMenu(_("Enabling cheats will not allow you to achieve 100% completion in the game"), &showCheatMenu);

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
