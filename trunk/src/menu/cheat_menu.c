/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../draw.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/pak.h"
#include "cheat_menu.h"
#include "label.h"
#include "options_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;
static int healthCheat, arrowCheat, lavaCheat;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static void toggleInfiniteHealth(void);
static void toggleInfiniteArrows(void);
static void toggleLavaNotFatal(void);
static void realignGrid(void);

void drawCheatMenu()
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
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 0;
		}

		menuInput.down = FALSE;
		input.down = FALSE;

		playSound("sound/common/click.ogg");
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

		playSound("sound/common/click.ogg");
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

		playSound("sound/common/click.ogg");
	}

	else if (input.left == TRUE || menuInput.left == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			w->leftAction();
		}

		menuInput.left = FALSE;
		input.left = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.right == TRUE || menuInput.right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->rightAction != NULL)
		{
			w->rightAction();
		}

		menuInput.right = FALSE;
		input.right = FALSE;

		playSound("sound/common/click.ogg");
	}
}

static void loadMenuLayout()
{
	char *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i;

	savePtr1 = NULL;

	i = 0;

	buffer = loadFileFromPak("data/menu/cheat_menu.dat");

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
				showErrorAndExit("Ran out of memory when creating Cheat Menu");
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "INFINITE_HEALTH") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, &toggleInfiniteHealth, &toggleInfiniteHealth, &toggleInfiniteHealth, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->label = createLabel(game.infiniteEnergy == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					healthCheat = game.infiniteEnergy;
				}

				else if (strcmpignorecase(menuID, "INFINITE_ARROWS") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, &toggleInfiniteArrows, &toggleInfiniteArrows, &toggleInfiniteArrows, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->label = createLabel(game.infiniteArrows == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					arrowCheat = game.infiniteArrows;
				}

				else if (strcmpignorecase(menuID, "LAVA_NOT_FATAL") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, &toggleLavaNotFatal, &toggleLavaNotFatal, &toggleLavaNotFatal, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->label = createLabel(game.lavaNotFatal == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					lavaCheat = game.lavaNotFatal;
				}

				else if (strcmpignorecase(menuID, "MENU_BACK") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showOptionsMenu, x, y, TRUE, 255, 255, 255);
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

	realignGrid();
}

Menu *initCheatMenu()
{
	menu.action = &doMenu;

	freeCheatMenu();

	loadMenuLayout();

	menu.returnAction = &showOptionsMenu;

	return &menu;
}

static void realignGrid()
{
	int i, maxWidth = 0;

	if (menu.widgets != NULL)
	{
		for (i=0;i<menu.widgetCount;i++)
		{
			if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth)
			{
				maxWidth = menu.widgets[i]->normalState->w;
			}
		}

		for (i=0;i<menu.widgetCount;i++)
		{
			if (menu.widgets[i]->label != NULL)
			{
				menu.widgets[i]->label->x = menu.widgets[i]->x + maxWidth + 10;
			}
		}
	}
}

void freeCheatMenu()
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

static void toggleInfiniteHealth()
{
	Widget *w = menu.widgets[menu.index];

	healthCheat = healthCheat == TRUE ? FALSE : TRUE;

	updateLabelText(w->label, healthCheat == TRUE ? _("Yes") : _("No"));
}

static void toggleInfiniteArrows()
{
	Widget *w = menu.widgets[menu.index];

	arrowCheat = arrowCheat == TRUE ? FALSE : TRUE;

	updateLabelText(w->label, arrowCheat == TRUE ? _("Yes") : _("No"));
}

static void toggleLavaNotFatal()
{
	Widget *w = menu.widgets[menu.index];

	lavaCheat = lavaCheat == TRUE ? FALSE : TRUE;

	updateLabelText(w->label, lavaCheat == TRUE ? _("Yes") : _("No"));
}

static void showOptionsMenu()
{
	game.infiniteEnergy = healthCheat;

	game.infiniteArrows = arrowCheat;

	game.lavaNotFatal = lavaCheat;

	if (healthCheat == TRUE || arrowCheat == TRUE || lavaCheat == TRUE)
	{
		game.cheating = TRUE;
	}

	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}
