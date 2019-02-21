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
#include "../graphics/texture_cache.h"
#include "../hud.h"
#include "../init.h"
#include "../medal.h"
#include "../system/error.h"
#include "label.h"
#include "main_menu.h"
#include "medals_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void doMenu(void);
static void showMainMenu(void);

void drawMedalsMenu()
{
	int i;
	SDL_Rect rect;

	drawImage(menu.background, menu.x, menu.y, FALSE, 196);

	rect.x = menu.x + 5;
	rect.y = menu.y + 5;
	rect.w = menu.w;
	rect.h = menu.h;

	SDL_RenderSetClipRect(game.renderer, &rect);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], &menu, -1);
	}

	SDL_RenderSetClipRect(game.renderer, NULL);
}

static void doMenu()
{
	int up, down, attack, block, xAxisMoved, yAxisMoved;

	up = FALSE;
	down = FALSE;
	attack = FALSE;
	block = FALSE;

	if (menuInput.up == TRUE)
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

	else if (menuInput.block == TRUE)
	{
		block = TRUE;
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

	else if (input.block == TRUE)
	{
		attack = TRUE;
	}

	xAxisMoved = input.xAxisMoved;
	yAxisMoved = input.yAxisMoved;

	if (attack == TRUE)
	{
		memset(&menuInput, 0, sizeof(Input));
		memset(&input, 0, sizeof(Input));

		playSound("sound/common/click");

		showMainMenu();
	}

	else if (block == TRUE)
	{
		memset(&menuInput, 0, sizeof(Input));
		memset(&input, 0, sizeof(Input));

		playSound("sound/common/click");

		showMainMenu();
	}

	else if (down == TRUE)
	{
		menu.startY += 6;

		if (menu.startY > menu.endY - menu.h)
		{
			menu.startY = menu.endY - menu.h;
		}
	}

	else if (up == TRUE)
	{
		menu.startY -= 6;

		if (menu.startY < 0)
		{
			menu.startY = 0;
		}
	}

	input.xAxisMoved = xAxisMoved;
	input.yAxisMoved = yAxisMoved;
}

static void loadMenuLayout()
{
	Medal *medal;
	int i, width, medalCount;
	Texture *texture;

	medal = getMedals();

	medalCount = getMedalCount();

	i = 0;

	width = 0;

	menu.w = 0;

	menu.h = 0;

	menu.startY = 0;

	menu.endY = 0;

	menu.widgetCount = medalCount;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Medals Menu");
	}

	for (i=0;i<menu.widgetCount;i++)
	{
		if (medal[i].hidden == TRUE && medal[i].obtained == FALSE)
		{
			menu.widgets[i] = createWidget(_("Hidden Medal"), NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
		}

		else
		{
			if (medal[i].obtained == TRUE)
			{
				menu.widgets[i] = createWidget(_(medal[i].description), NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 0, 200, 0);
			}

			else
			{
				menu.widgets[i] = createWidget(_(medal[i].description), NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			}
		}

		if (width < menu.widgets[i]->x + menu.widgets[i]->normalState->w)
		{
			width = menu.widgets[i]->x + menu.widgets[i]->normalState->w;
		}

		texture = getMedalImage(medal[i].medalType, medal[i].obtained);

		menu.widgets[i]->label = createImageLabel(texture, menu.widgets[i]->x, menu.widgets[i]->y);

		menu.widgets[i]->label->y = menu.widgets[i]->y + menu.widgets[i]->normalState->h / 2 - menu.widgets[i]->label->text->h / 2;

		menu.endY = menu.widgets[i]->y + menu.widgets[i]->normalState->h - menu.h;
	}

	width += 15;

	menu.h = SCREEN_HEIGHT - BUTTON_PADDING;

	for (i=0;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->label != NULL)
		{
			menu.widgets[i]->label->x = width;

			if (menu.w < menu.widgets[i]->label->x + menu.widgets[i]->label->text->w)
			{
				menu.w = menu.widgets[i]->label->x + menu.widgets[i]->label->text->w;
			}
		}
	}

	menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initMedalsMenu()
{
	menu.action = &doMenu;

	freeMedalsMenu();

	loadMenuLayout();

	menu.index = 0;

	menu.returnAction = &showMainMenu;

	return &menu;
}

void freeMedalsMenu()
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
