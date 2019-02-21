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
#include "widget.h"
#include "yes_no_menu.h"

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

void drawYesNoMenu()
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
	int left, right, attack, xAxisMoved, yAxisMoved;

	left = FALSE;
	right = FALSE;
	attack = FALSE;

	if (menuInput.left == TRUE)
	{
		left = TRUE;
	}

	else if (menuInput.right == TRUE)
	{
		right = TRUE;
	}

	else if (menuInput.attack == TRUE)
	{
		attack = TRUE;
	}

	else if (input.left == TRUE)
	{
		left = TRUE;
	}

	else if (input.right == TRUE)
	{
		right = TRUE;
	}

	else if (input.attack == TRUE)
	{
		attack = TRUE;
	}

	if (right == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 2;
		}

		playSound("sound/common/click");
	}

	else if (left == TRUE)
	{
		menu.index--;

		if (menu.index < 2)
		{
			menu.index = menu.widgetCount - 1;
		}

		playSound("sound/common/click");
	}

	else if (attack == TRUE)
	{
		w = menu.widgets[menu.index];

		playSound("sound/common/click");

		if (w->clickAction != NULL)
		{
			w->clickAction();
		}
	}

	xAxisMoved = input.xAxisMoved;
	yAxisMoved = input.yAxisMoved;

	memset(&menuInput, 0, sizeof(Input));
	memset(&input, 0, sizeof(Input));

	input.xAxisMoved = xAxisMoved;
	input.yAxisMoved = yAxisMoved;
}

static void loadMenuLayout(char *text)
{
	int x, y;

	x = 0;
	y = 0;

	menu.widgetCount = 4;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Yes / No Menu");
	}

	menu.widgets[0] = createWidget(text, NULL, NULL, NULL, NULL, -1, 20, FALSE, 255, 255, 255);

	menu.widgets[1] = createWidget(_("Any unsaved progress will be lost"), NULL, NULL, NULL, NULL, -1, 70, FALSE, 255, 255, 255);

	menu.widgets[2] = createWidget(_("Yes"), NULL, NULL, NULL, &doYes, x, y, TRUE, 255, 255, 255);

	menu.widgets[3] = createWidget(_("No"), NULL, NULL, NULL, &doNo, x, y, TRUE, 255, 255, 255);

	/* Resize */

	menu.widgets[0]->y = BORDER_PADDING + BUTTON_PADDING;
	menu.widgets[1]->y = menu.widgets[0]->y + menu.widgets[0]->selectedState->h + BUTTON_PADDING;

	menu.widgets[2]->y = menu.widgets[1]->y + menu.widgets[1]->selectedState->h + BUTTON_PADDING;
	menu.widgets[3]->y = menu.widgets[2]->y;

	if (menu.widgets[0]->selectedState->w > menu.widgets[1]->selectedState->w)
	{
		menu.w = menu.widgets[0]->selectedState->w;
	}

	else
	{
		menu.w = menu.widgets[1]->selectedState->w;
	}

	menu.h = menu.widgets[2]->y + menu.widgets[2]->selectedState->h + BUTTON_PADDING - BORDER_PADDING;

	x = menu.widgets[2]->normalState->w + BUTTON_PADDING + menu.widgets[3]->normalState->w;

	menu.widgets[2]->x = (menu.w - x) / 2;
	menu.widgets[3]->x = menu.widgets[2]->x + menu.widgets[2]->selectedState->w + BUTTON_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initYesNoMenu(char *text, void (*yes)(void), void (*no)(void))
{
	menu.action = &doMenu;

	yesAction = yes;
	noAction = no;

	freeYesNoMenu();

	loadMenuLayout(text);

	menu.index = 3;

	menu.returnAction = NULL;

	return &menu;
}

void freeYesNoMenu()
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
