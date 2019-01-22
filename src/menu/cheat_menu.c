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
static void toggleLavaIsFatal(void);

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
	int left, right, up, down, attack, xAxisMoved, yAxisMoved;

	left = FALSE;
	right = FALSE;
	up = FALSE;
	down = FALSE;
	attack = FALSE;

	if (menuInput.left == TRUE)
	{
		left = TRUE;
	}

	else if (menuInput.right == TRUE)
	{
		right = TRUE;
	}

	else if (menuInput.up == TRUE)
	{
		up = TRUE;
	}

	else if (menuInput.down == TRUE)
	{
		down = TRUE;
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

	else if (input.up == TRUE)
	{
		up = TRUE;
	}

	else if (input.down == TRUE)
	{
		down = TRUE;
	}

	else if (input.attack == TRUE)
	{
		attack = TRUE;
	}

	if (down == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 0;
		}

		playSound("sound/common/click");
	}

	else if (up == TRUE)
	{
		menu.index--;

		if (menu.index < 0)
		{
			menu.index = menu.widgetCount - 1;
		}

		playSound("sound/common/click");
	}

	else if (attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->clickAction != NULL)
		{
			w->clickAction();
		}

		playSound("sound/common/click");
	}

	else if (left == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			w->leftAction();
		}

		playSound("sound/common/click");
	}

	else if (right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->rightAction != NULL)
		{
			w->rightAction();
		}

		playSound("sound/common/click");
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
	int i, x, y, w, maxWidth;

	menu.widgetCount = 4;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Cheat Menu");
	}

	x = 20;

	y = 0;

	menu.widgets[0] = createWidget(_("Infinite Health"), NULL, &toggleInfiniteHealth, &toggleInfiniteHealth, &toggleInfiniteHealth, x, y, TRUE, 255, 255, 255);

	menu.widgets[0]->label = createLabel(game.infiniteEnergy == TRUE ? _("Yes") : _("No"), menu.widgets[0]->x + menu.widgets[0]->normalState->w + 10, y);

	healthCheat = game.infiniteEnergy;

	menu.widgets[1] = createWidget(_("Infinite Arrows"), NULL, &toggleInfiniteArrows, &toggleInfiniteArrows, &toggleInfiniteArrows, x, y, TRUE, 255, 255, 255);

	menu.widgets[1]->label = createLabel(game.infiniteArrows == TRUE ? _("Yes") : _("No"), menu.widgets[1]->x + menu.widgets[1]->normalState->w + 10, y);

	arrowCheat = game.infiniteArrows;

	menu.widgets[2] = createWidget(_("Lava is fatal"), NULL, &toggleLavaIsFatal, &toggleLavaIsFatal, &toggleLavaIsFatal, x, y, TRUE, 255, 255, 255);

	menu.widgets[2]->label = createLabel(game.lavaIsFatal == TRUE ? _("Yes") : _("No"), menu.widgets[2]->x + menu.widgets[2]->normalState->w + 10, y);

	lavaCheat = game.lavaIsFatal == TRUE ? FALSE : TRUE;

	menu.widgets[3] = createWidget(_("Back"), NULL, NULL, NULL, &showOptionsMenu, -1, y, TRUE, 255, 255, 255);

	y = BUTTON_PADDING + BORDER_PADDING;

	maxWidth = w = 0;

	for (i=0;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth)
		{
			maxWidth = menu.widgets[i]->normalState->w;
		}
	}

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

		y += menu.widgets[i]->selectedState->h + BUTTON_PADDING;
	}

	menu.w = w + BUTTON_PADDING;
	menu.h = y - BORDER_PADDING;

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initCheatMenu()
{
	menu.action = &doMenu;

	freeCheatMenu();

	loadMenuLayout();

	menu.returnAction = &showOptionsMenu;

	return &menu;
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

		menu.widgets = NULL;
	}

	if (menu.background != NULL)
	{
		destroyTexture(menu.background);

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

static void toggleLavaIsFatal()
{
	Widget *w = menu.widgets[menu.index];

	lavaCheat = lavaCheat == TRUE ? FALSE : TRUE;

	updateLabelText(w->label, lavaCheat == FALSE ? _("Yes") : _("No"));
}

static void showOptionsMenu()
{
	game.infiniteEnergy = healthCheat;

	game.infiniteArrows = arrowCheat;

	game.lavaIsFatal = lavaCheat == TRUE ? FALSE : TRUE;

	if (healthCheat == TRUE || arrowCheat == TRUE || lavaCheat == TRUE)
	{
		game.cheating = TRUE;
	}

	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}
