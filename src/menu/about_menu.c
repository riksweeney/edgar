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
#include "../draw.h"
#include "label.h"
#include "../init.h"
#include "../graphics/graphics.h"
#include "../system/pak.h"
#include "../input.h"
#include "../audio/audio.h"
#include "main_menu.h"
#include "../system/error.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void doMenu(void);
static void showMainMenu(void);

void drawAboutMenu()
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
	if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		menuInput.attack = FALSE;
		input.attack = FALSE;

		playSound("sound/common/click.ogg");

		showMainMenu();
	}
}

static void loadMenuLayout()
{
	char versionText[MAX_VALUE_LENGTH];

	menu.widgetCount = 3;

	menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

	snprintf(versionText, MAX_VALUE_LENGTH, "The Legend of Edgar v%0.2f", VERSION);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating About Menu");
	}

	menu.widgets[0] = createWidget(versionText, NULL, NULL, NULL, NULL, -1, 20, FALSE);

	menu.widgets[1] = createWidget(_("Copyright Parallel Realities 2009"), NULL, NULL, NULL, NULL, -1, 70, FALSE);

	menu.widgets[2] = createWidget(_("OK"), NULL, NULL, NULL, NULL, -1, 120, TRUE);

	/* Resize */

	if (menu.widgets[0]->selectedState->w > menu.widgets[1]->selectedState->w)
	{
		menu.w = menu.widgets[0]->selectedState->w + 20;
	}

	else
	{
		menu.w = menu.widgets[1]->selectedState->w + 20;
	}

	menu.h = 170;

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initAboutMenu()
{
	menu.action = &doMenu;

	loadMenuLayout();

	menu.index = 2;

	menu.returnAction = &showMainMenu;

	return &menu;
}

void freeAboutMenu()
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
