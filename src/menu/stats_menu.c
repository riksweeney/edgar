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
#include "../game.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "main_menu.h"
#include "stats_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void doMenu(void);
static void showMainMenu(void);

void drawStatsMenu()
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
	int xAxisMoved, yAxisMoved;

	if (menuInput.attack == TRUE || input.attack == TRUE)
	{
		playSound("sound/common/click");

		showMainMenu();
	}

	xAxisMoved = input.xAxisMoved;
	yAxisMoved = input.yAxisMoved;

	memset(&menuInput, 0, sizeof(Input));
	memset(&input, 0, sizeof(Input));

	input.xAxisMoved = xAxisMoved;
	input.yAxisMoved = yAxisMoved;
}

static void loadMenuLayout()
{
	char menuName[MAX_VALUE_LENGTH], *token;
	int w, y, i, maxWidth;
	float distance;

	w = 0;

	i = 0;

	maxWidth = 0;

	menu.widgetCount = 11;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Stats Menu");
	}

	menu.widgets[0] = createWidget(_("Statistics"), NULL, NULL, NULL, NULL, -1, 0, TRUE, 255, 255, 255);

	token = getPlayTimeAsString();

	snprintf(menuName, MAX_VALUE_LENGTH, _("Play Time: %s"), token);

	free(token);

	menu.widgets[1] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	snprintf(menuName, MAX_VALUE_LENGTH, _("Kills: %d"), game.kills);

	menu.widgets[2] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	snprintf(menuName, MAX_VALUE_LENGTH, _("Arrows Fired: %d"), game.arrowsFired);

	menu.widgets[3] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	snprintf(menuName, MAX_VALUE_LENGTH, _("Bats Drowned: %d"), game.batsDrowned);

	menu.widgets[4] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	snprintf(menuName, MAX_VALUE_LENGTH, _("Times Eaten: %d"), game.timesEaten);

	menu.widgets[5] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	distance = game.distanceTravelled;

	distance /= 45000; /* 45 pixels is 1 metre */

	snprintf(menuName, MAX_VALUE_LENGTH, _("Distanced Travelled: %0.1fKM"), distance);

	menu.widgets[6] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	snprintf(menuName, MAX_VALUE_LENGTH, _("Attacks Blocked: %d"), game.attacksBlocked);

	menu.widgets[7] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	token = getSlimeTimeAsString();

	snprintf(menuName, MAX_VALUE_LENGTH, _("Time Spent As A Slime: %s"), token);

	free(token);

	menu.widgets[8] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	snprintf(menuName, MAX_VALUE_LENGTH, _("Secrets Found: %d / %d"), game.secretsFound, TOTAL_SECRETS);

	menu.widgets[9] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 0, FALSE, 255, 255, 255);

	menu.widgets[10] = createWidget(_("OK"), NULL, NULL, NULL, NULL, -1, 0, TRUE, 255, 255, 255);

	y = BUTTON_PADDING / 2 + BORDER_PADDING;

	for (i=0;i<menu.widgetCount;i++)
	{
		menu.widgets[i]->y = y;

		if (menu.widgets[i]->x != -1)
		{
			menu.widgets[i]->x = BUTTON_PADDING / 2 + BORDER_PADDING;
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

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initStatsMenu()
{
	menu.action = &doMenu;

	freeStatsMenu();

	loadMenuLayout();

	menu.index = 10;

	menu.returnAction = &showMainMenu;

	return &menu;
}

void freeStatsMenu()
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
