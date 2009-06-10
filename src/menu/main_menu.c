/*
Copyright (C) 2009 Parallel Realities

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

#include "widget.h"
#include "../init.h"
#include "../graphics/graphics.h"
#include "options_menu.h"
#include "../system/pak.h"
#include "../system/load_save.h"
#include "../game.h"

extern Input input, menuInput;
extern Game game;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static void showSaveDialog(void);
static void doNewGame(void);

void drawMainMenu()
{
	int i;

	drawImage(menu.background, menu.x, menu.y, FALSE);

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
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 0;
		}

		menuInput.down = FALSE;
		input.down = FALSE;
	}

	else if (input.up == TRUE || menuInput.up == TRUE)
	{
		menu.index--;

		if (menu.index < 0)
		{
			menu.index = menu.widgetCount - 1;
		}

		menuInput.up = FALSE;
		input.up = FALSE;
	}

	else if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->clickAction != NULL)
		{
			w->clickAction();
		}

		menuInput.attack = FALSE;
		input.attack = FALSE;
	}

	else if (input.left == TRUE || menuInput.left == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->rightAction != NULL)
		{
			w->rightAction();
		}

		menuInput.left = FALSE;
		input.left = FALSE;
	}

	else if (input.right == TRUE || menuInput.right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			w->leftAction();
		}

		menuInput.right = FALSE;
		input.right = FALSE;
	}
}

static void loadMenuLayout()
{
	char filename[MAX_LINE_LENGTH], *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i;
	SDL_Surface *temp;

	i = 0;

	snprintf(filename, sizeof(filename), _("data/menu/main_menu.dat"));

	buffer = loadFileFromPak(filename);

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

			menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				printf("Ran out of memory when creating Main Menu\n");

				exit(1);
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
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, &doNewGame, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_LOAD") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, &showSaveDialog, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_OPTIONS") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, &showOptionsMenu, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_ABOUT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, NULL, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_QUIT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, &quitGame, x, y);
				}

				else
				{
					printf("Unknown widget %s\n", menuID);

					exit(1);
				}

				i++;
			}

			else
			{
				printf("Widget Count must be defined!\n");

				exit(1);
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w == 0 || menu.h == 0)
	{
		printf("Menu dimensions must be greater than 0\n");

		exit(1);
	}

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, menu.w, menu.h, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	menu.background = addBorder(SDL_DisplayFormat(temp), 255, 255, 255, 0, 0, 0);

	SDL_FreeSurface(temp);

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
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	menu.returnAction = NULL;

	return &menu;
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}

static void showSaveDialog()
{
	if (loadGame(0) == TRUE)
	{
		pauseGame();
	}
}

static void doNewGame()
{
	newGame();
	
	pauseGame();
}
