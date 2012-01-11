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
#include "../hud.h"
#include "../init.h"
#include "../medal.h"
#include "../system/error.h"
#include "../system/pak.h"
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
	
	SDL_SetClipRect(game.screen, &rect);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], &menu, -1);
	}
	
	SDL_SetClipRect(game.screen, NULL);
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
	
	else if (input.block == TRUE || menuInput.block == TRUE)
	{
		menuInput.block = FALSE;
		input.block = FALSE;

		playSound("sound/common/click.ogg");

		showMainMenu();
	}
	
	else if (input.down == TRUE || menuInput.down == TRUE)
	{
		menu.startY += 6;
		
		if (menu.startY > menu.endY)
		{
			menu.startY = menu.endY;
		}
	}
	
	else if (input.up == TRUE || menuInput.up == TRUE)
	{
		menu.startY -= 6;
		
		if (menu.startY < 0)
		{
			menu.startY = 0;
		}
	}
}

static void loadMenuLayout()
{
	char *line, *token, *savePtr1, *savePtr2;
	Medal *medal;
	unsigned char *buffer;
	int i, width, medalCount;

	savePtr1 = NULL;
	
	medal = getMedals();
	
	medalCount = getMedalCount();

	i = 0;
	
	width = 0;

	buffer = loadFileFromPak("data/menu/medals_menu.dat");

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	while (line != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr1);

			continue;
		}

		token = strtok_r(line, " ", &savePtr2);

		if (strcmpignorecase(token, "WIDTH") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.w = atoi(token);
		}

		else if (strcmpignorecase(token, "HEIGHT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.h = atoi(token);
		}
		
		line = strtok_r(NULL, "\n", &savePtr1);
	}

	menu.widgetCount = medalCount;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Medals Menu");
	}

	for (i=0;i<menu.widgetCount;i++)	{

		if (medal[i].hidden == TRUE && medal[i].obtained == FALSE)
		{
			menu.widgets[i] = createWidget(_("Hidden Medal"), NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
		}
		
		else
		{
			if (medal[i].obtained == TRUE)
			{
				menu.widgets[i] = createWidget(medal[i].description, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 0, 200, 0);
			}
			
			else
			{
				menu.widgets[i] = createWidget(medal[i].description, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			}
		}
		
		if (width < menu.widgets[i]->x + menu.widgets[i]->normalState->w)
		{
			width = menu.widgets[i]->x + menu.widgets[i]->normalState->w;
		}
		
		menu.widgets[i]->label = createImageLabel(getMedalImage(medal[i].medalType, medal[i].obtained), menu.widgets[i]->x, menu.widgets[i]->y);
		
		menu.widgets[i]->label->y = menu.widgets[i]->y + menu.widgets[i]->normalState->h / 2 - menu.widgets[i]->label->text->h / 2;
		
		menu.endY = menu.widgets[i]->y + menu.widgets[i]->normalState->h - menu.h;
	}

	line = strtok_r(NULL, "\n", &savePtr1);
	
	width += 15;
	
	menu.w = 0;
	
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

	if (menu.w <= 0 || menu.h <= 0)
	{
		showErrorAndExit("Menu dimensions must be greater than 0");
	}

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	free(buffer);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initMedalsMenu()
{
	menu.action = &doMenu;

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
