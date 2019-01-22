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
#include "script_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;
static void (*yesAction)(void);
static void (*noAction)(void);

static void loadMenuLayout(char *);
static void doMenu(void);
static void doYes(void);
static void doNo(void);

void drawScriptMenu()
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

	if (input.right == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 1;
		}

		input.right = FALSE;

		playSound("sound/common/click");
	}

	else if (input.left == TRUE)
	{
		menu.index--;

		if (menu.index < 1)
		{
			menu.index = menu.widgetCount - 1;
		}

		input.left = FALSE;

		playSound("sound/common/click");
	}

	else if (input.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		input.attack = FALSE;

		playSound("sound/common/click");

		if (w->clickAction != NULL)
		{
			w->clickAction();
		}
	}
}

static void loadMenuLayout(char *text)
{
	int x, y;

	x = 20;

	y = 0;

	menu.widgetCount = 3;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Script Menu");
	}

	menu.widgets[0] = createWidget(_(text), NULL, NULL, NULL, NULL, -1, 20, FALSE, 255, 255, 255);

	menu.widgets[1] = createWidget(_("Yes"), NULL, NULL, NULL, doYes, x, y, TRUE, 255, 255, 255);

	menu.widgets[2] = createWidget(_("No"), NULL, NULL, NULL, doNo, x, y, TRUE, 255, 255, 255);

	/* Resize */

	menu.widgets[0]->y = BORDER_PADDING + BUTTON_PADDING;

	menu.widgets[1]->y = menu.widgets[0]->y + menu.widgets[0]->selectedState->h + BUTTON_PADDING;
	menu.widgets[2]->y = menu.widgets[1]->y;

	menu.w = menu.widgets[0]->selectedState->w;

	if (menu.w < 150)
	{
		menu.w = 150;
	}

	menu.h = menu.widgets[2]->y + menu.widgets[2]->selectedState->h + BUTTON_PADDING - BORDER_PADDING;

	x = menu.widgets[1]->normalState->w + BUTTON_PADDING + menu.widgets[2]->normalState->w;

	menu.widgets[1]->x = (menu.w - x) / 2;
	menu.widgets[2]->x = menu.widgets[1]->x + menu.widgets[1]->selectedState->w + BUTTON_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initScriptMenu(char *text, void (*yes)(void), void (*no)(void))
{
	menu.action = &doMenu;

	yesAction = yes;
	noAction = no;

	freeScriptMenu();

	loadMenuLayout(text);

	menu.index = 2;

	menu.returnAction = NULL;

	return &menu;
}

void freeScriptMenu()
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

static void doYes()
{
	yesAction();
}

static void doNo()
{
	noAction();
}
