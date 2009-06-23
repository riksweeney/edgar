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
#include "label.h"
#include "../init.h"
#include "../graphics/graphics.h"
#include "main_menu.h"
#include "io_menu.h"
#include "sound_menu.h"
#include "../game.h"
#include "../hud.h"
#include "../system/pak.h"
#include "../system/load_save.h"
#include "../audio/audio.h"

extern Input input, menuInput;
extern Game game;
extern Entity *self;

static Menu menu;

static void loadMenuLayout(int);
static void showMainMenu(void);
static void doMenu(void);
static void saveGameInSlot(void);
static void loadGameInSlot(void);

void drawIOMenu()
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
			playSound("sound/common/click.ogg");

			w->clickAction();
		}

		menuInput.attack = FALSE;
		input.attack = FALSE;
	}
}

static void loadMenuLayout(int saving)
{
	char filename[MAX_LINE_LENGTH], *line, *token, *savePtr1, *savePtr2;
	char **saveFile;
	unsigned char *buffer;
	int x, y, i;

	i = 0;

	snprintf(filename, sizeof(filename), _("data/menu/io_menu.dat"));

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

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	menu.widgetCount = MAX_SAVE_SLOTS + 1;

	menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		printf("Ran out of memory when creating IO Menu\n");

		exit(1);
	}

	if (menu.w <= 0 || menu.h <= 0)
	{
		printf("Menu dimensions must be greater than 0\n");

		exit(1);
	}

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	SDL_SetAlpha(menu.background, SDL_SRCALPHA|SDL_RLEACCEL, 196);

	free(buffer);

	saveFile = getSaveFileIndex();

	x = y = 5;

	for (i=0;i<MAX_SAVE_SLOTS;i++)
	{
		if (saveFile == NULL || strlen(saveFile[i]) == 0)
		{
			menu.widgets[i] = createWidget("<Empty>", NULL, NULL, NULL, saving == TRUE ? &saveGameInSlot : NULL, -1, y, FALSE);
		}

		else
		{
			menu.widgets[i] = createWidget(saveFile[i], NULL, NULL, NULL, saving == TRUE ? &saveGameInSlot : &loadGameInSlot, -1, y, FALSE);
		}

		y += menu.widgets[i]->normalState->h + 5;
	}

	y += 15;

	menu.widgets[MAX_SAVE_SLOTS] = createWidget(_("Back"), NULL, 0, 0, &showMainMenu, -1, y, TRUE);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initIOMenu(int saving)
{
	menu.action = &doMenu;

	freeIOMenu();

	loadMenuLayout(saving);

	menu.returnAction = saving == TRUE ? NULL : &showMainMenu;

	return &menu;
}

void freeIOMenu()
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

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}

static void loadGameInSlot()
{
	if (loadGame(menu.index) == TRUE)
	{
		menu.returnAction = NULL;

		freeMessageQueue();

		setInfoBoxMessage(120, _("Game Loaded"));

		pauseGame();
	}
}

static void saveGameInSlot()
{
	saveGame(menu.index);

	freeMessageQueue();

	setInfoBoxMessage(120, _("Game Saved"));

	pauseGame();
}
