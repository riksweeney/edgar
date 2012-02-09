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
#include "../game.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/load_save.h"
#include "io_menu.h"
#include "main_menu.h"
#include "ok_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Entity *self;

static Menu menu;

static void loadMenuLayout(int);
static void showMainMenu(void);
static void doMenu(void);
static void saveGameInSlot(void);
static void loadGameInSlot(void);
static void showIOMenu(void);

void drawIOMenu()
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

	if (menuInput.down == TRUE || input.down == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 1;
		}

		playSound("sound/common/click.ogg");
	}

	else if (menuInput.up == TRUE || input.up == TRUE)
	{
		menu.index--;

		if (menu.index < 1)
		{
			menu.index = menu.widgetCount - 1;
		}

		playSound("sound/common/click.ogg");
	}

	else if (menuInput.attack == TRUE || input.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->clickAction != NULL)
		{
			playSound("sound/common/click.ogg");

			w->clickAction();
		}
	}

	memset(&menuInput, 0, sizeof(Input));
	memset(&input, 0, sizeof(Input));
}

static void loadMenuLayout(int saving)
{
	char **saveFile;
	int y, i, j, width , maxWidth, w;

	w = i = 0;

	y = BUTTON_PADDING / 2 + BORDER_PADDING;

	menu.widgetCount = MAX_SAVE_SLOTS + 2;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating IO Menu");
	}

	saveFile = getSaveFileIndex();

	if (saving == TRUE)
	{
		menu.widgets[0] = createWidget(_("Choose slot to save to"), NULL, NULL, NULL, NULL, -1, y, FALSE, 255, 255, 255);
	}

	else
	{
		menu.widgets[0] = createWidget(_("Choose slot to load from"), NULL, NULL, NULL, NULL, -1, y, FALSE, 255, 255, 255);
	}

	maxWidth = 0;

	i = 1;

	for (j=0;j<MAX_SAVE_SLOTS;j++)
	{
		if (saveFile == NULL || strlen(saveFile[j]) == 0)
		{
			menu.widgets[i] = createWidget(_("<Empty>"), NULL, NULL, NULL, saving == TRUE ? &saveGameInSlot : NULL, -1, y, FALSE, 255, 255, 255);
		}

		else
		{
			menu.widgets[i] = createWidget(saveFile[j], NULL, NULL, NULL, saving == TRUE ? &saveGameInSlot : &loadGameInSlot, -1, y, FALSE, 255, 255, 255);
		}

		if (saveFile[j] != NULL)
		{
			free(saveFile[j]);
		}

		if (menu.widgets[i]->selectedState->w > width)
		{
			width = menu.widgets[i]->selectedState->w;
		}

		i++;
	}

	menu.widgets[i] = createWidget(_("Back"), NULL, 0, 0, &showMainMenu, -1, y, TRUE, 255, 255, 255);

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

		y += menu.widgets[i]->selectedState->h + BUTTON_PADDING / 2;
	}

	menu.w = w + BUTTON_PADDING;
	menu.h = y - BORDER_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initIOMenu(int saving)
{
	menu.action = &doMenu;

	freeIOMenu();

	loadMenuLayout(saving);

	menu.returnAction = saving == TRUE ? NULL : &showMainMenu;

	if (menu.index == 0)
	{
		menu.index = 1;
	}

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
	if (loadGame(menu.index - 1) == TRUE)
	{
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

static void saveGameInSlot()
{
	saveGame(menu.index - 1);

	freeMessageQueue();

	setInfoBoxMessage(60, 255, 255, 255, _("Game Saved"));

	pauseGame();
}

static void showIOMenu()
{
	game.menu = initIOMenu(FALSE);

	game.drawMenu = &drawIOMenu;
}
