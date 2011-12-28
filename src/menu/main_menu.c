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
#include "../game.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/load_save.h"
#include "../system/pak.h"
#include "about_menu.h"
#include "io_menu.h"
#include "medals_menu.h"
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

		while (menu.widgets[menu.index]->disabled == TRUE);

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

		while (menu.widgets[menu.index]->disabled == TRUE);

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

	buffer = loadFileFromPak("data/menu/main_menu.dat");

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
				showErrorAndExit("Ran out of memory when creating Main Menu");
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "MENU_NEW_GAME") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &doNewGame, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_CONTINUE") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &getContinuePoint, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->disabled = game.canContinue == TRUE ? FALSE : TRUE;
				}

				else if (strcmpignorecase(menuID, "MENU_TUTORIAL") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &doTutorial, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_LOAD") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showIOMenu, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_OPTIONS") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showOptionsMenu, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_STATS") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showStatsMenu, x, y, TRUE, 255, 255, 255);
				}
				
				else if (strcmpignorecase(menuID, "MENU_MEDALS") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showMedalsMenu, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_ABOUT") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showAboutMenu, x, y, TRUE, 255, 255, 255);
				}

				else if (strcmpignorecase(menuID, "MENU_QUIT") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &doQuit, x, y, TRUE, 255, 255, 255);
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
	}

	if (menu.background != NULL)
	{
		SDL_FreeSurface(menu.background);

		menu.background = NULL;
	}
}

Menu *initMainMenu()
{
	int i;

	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	for (i=0;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->clickAction == &getContinuePoint)
		{
			menu.widgets[i]->disabled = game.canContinue == TRUE ? FALSE : TRUE;

			if (menu.widgets[i]->disabled == TRUE && menu.index == i)
			{
				menu.index = 0;
			}
		}
	}

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
	game.menu = initYesNoMenu(_("Start a new game?"), &newGame, &showMainMenu);

	game.drawMenu = &drawYesNoMenu;
}

static void doTutorial()
{
	game.menu = initYesNoMenu(_("Play the tutorial?"), &tutorial, &showMainMenu);

	game.drawMenu = &drawYesNoMenu;
}

static void doQuit()
{
	game.menu = initYesNoMenu(_("Exit the game?"), &quitGame, &showMainMenu);

	game.drawMenu = &drawYesNoMenu;
}

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}
