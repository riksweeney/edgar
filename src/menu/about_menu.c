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
#include "../draw.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "about_menu.h"
#include "main_menu.h"
#include "widget.h"

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
	if (menuInput.attack == TRUE || input.attack == TRUE)
	{
		playSound("sound/common/click");

		showMainMenu();
	}

	memset(&menuInput, 0, sizeof(Input));
	memset(&input, 0, sizeof(Input));
}

static void loadMenuLayout()
{
	int y;
	char versionText[MAX_VALUE_LENGTH], copyright[MAX_VALUE_LENGTH];

	menu.widgetCount = 3;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	snprintf(versionText, MAX_VALUE_LENGTH, _("The Legend of Edgar v%0.2f"), VERSION);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating About Menu");
	}

	y = BUTTON_PADDING + BORDER_PADDING;

	menu.widgets[0] = createWidget(versionText, NULL, NULL, NULL, NULL, -1, y, FALSE, 255, 255, 255);

	y += menu.widgets[0]->selectedState->h + BUTTON_PADDING;

	snprintf(copyright, MAX_VALUE_LENGTH, _("Copyright Parallel Realities 2009 - %d"), YEAR);

	menu.widgets[1] = createWidget(copyright, NULL, NULL, NULL, NULL, -1, y, FALSE, 255, 255, 255);

	y += menu.widgets[1]->selectedState->h + BUTTON_PADDING;

	menu.widgets[2] = createWidget(_("OK"), NULL, NULL, NULL, NULL, -1, y, TRUE, 255, 255, 255);

	y += menu.widgets[2]->selectedState->h + BUTTON_PADDING;

	/* Resize */

	if (menu.widgets[0]->selectedState->w > menu.widgets[1]->selectedState->w)
	{
		menu.w = menu.widgets[0]->selectedState->w;
	}

	else
	{
		menu.w = menu.widgets[1]->selectedState->w;
	}

	menu.h = y - BORDER_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initAboutMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

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

		menu.widgets = NULL;
	}

	if (menu.background != NULL)
	{
		destroyTexture(menu.background);

		menu.background = NULL;
	}
}

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}
