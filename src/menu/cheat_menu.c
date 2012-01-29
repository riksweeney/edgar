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
	
	menu.widgets[2] = createWidget(_("Lava is not fatal"), NULL, &toggleLavaNotFatal, &toggleLavaNotFatal, &toggleLavaNotFatal, x, y, TRUE, 255, 255, 255);

	menu.widgets[2]->label = createLabel(game.lavaNotFatal == TRUE ? _("Yes") : _("No"), menu.widgets[2]->x + menu.widgets[2]->normalState->w + 10, y);

	lavaCheat = game.lavaNotFatal;
	
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

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

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
