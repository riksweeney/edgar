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

#include "about_menu.h"
#include "cheat_menu.h"
#include "control_menu.h"
#include "inventory_menu.h"
#include "io_menu.h"
#include "main_menu.h"
#include "ok_menu.h"
#include "options_menu.h"
#include "sound_menu.h"
#include "stats_menu.h"
#include "yes_no_menu.h"

extern Game game;

void doMenu()
{
	game.menu->action();
}

void drawMenu()
{
	game.drawMenu();
}

void freeMenus()
{
	freeMainMenu();

	freeOptionsMenu();

	freeIOMenu();

	freeSoundMenu();

	freeControlMenu();

	freeYesNoMenu();

	freeStatsMenu();

	freeCheatMenu();

	freeInventoryMenu();

	freeOKMenu();

	freeAboutMenu();
}

void resizeMenu(Menu *menu)
{
	int i, width, height, widgetWidth, widgetHeight;
	Widget *widget;
	Label *label;

	width = -1;
	height = -1;

	for (i=0;i<menu.widgetCount;i++)
	{
		widget = menu.widgets[i]->selectedState;

		if (widget == NULL)
		{
			widget = menu.widgets[i]->normalState;
		}

		label = widget->label;

		if (label != NULL)
		{
			widgetWidth = label->text->x + label->text->w;

			widgetHeight = label->text->h;
		}

		else
		{
			widgetWidth = widget->w;

			widgetHeight = widget->h;
		}

		if (widgetWidth > width)
		{
			width = widgetWidth;
		}

		if (widgetHeight > height)
		{
			height = widgetHeight;
		}
	}

	menu->w = width;
	menu->h = height;
}
