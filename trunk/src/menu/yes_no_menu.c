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

	drawImage(menu.background, menu.x, menu.y, FALSE);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], &menu, menu.index == i);
	}
}

static void doMenu()
{
	Widget *w;

	if (input.right == TRUE || menuInput.right == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 2;
		}

		menuInput.right = FALSE;
		input.right = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.left == TRUE || menuInput.left == TRUE)
	{
		menu.index--;

		if (menu.index < 2)
		{
			menu.index = menu.widgetCount - 1;
		}

		menuInput.left = FALSE;
		input.left = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		menuInput.attack = FALSE;
		input.attack = FALSE;

		playSound("sound/common/click.ogg");

		if (w->clickAction != NULL)
		{
			w->clickAction();
		}
	}
}

static void loadMenuLayout(char *text)
{
	char filename[MAX_LINE_LENGTH], *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i;

	savePtr1 = NULL;

	i = 0;

	snprintf(filename, sizeof(filename), _("data/menu/yes_no_menu.dat"));

	buffer = loadFileFromPak(filename);

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

		else if (strcmpignorecase(token, "WIDGET_COUNT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.widgetCount = atoi(token) + 2;

			menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				printf("Ran out of memory when creating Yes / No Menu\n");

				exit(1);
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				if (i == 0)
				{
					menu.widgets[i++] = createWidget(text, NULL, NULL, NULL, NULL, -1, 20, FALSE);

					menu.widgets[i++] = createWidget(_("Any unsaved progress will be lost"), NULL, NULL, NULL, NULL, -1, 70, FALSE);
				}

				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "MENU_YES") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, doYes, x, y, TRUE);
				}

				else if (strcmpignorecase(menuID, "MENU_NO") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, doNo, x, y, TRUE);
				}

				else
				{
					printf("Unknown widget %s\n", menuID);

					exit(1);
				}

				i++;
			}

			else
			{
				printf("Widget Count must be defined!\n");

				exit(1);
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w <= 0 || menu.h <= 0)
	{
		printf("Menu dimensions must be greater than 0\n");

		exit(1);
	}

	/* Resize */

	if (menu.widgets[0]->selectedState->w > menu.widgets[1]->selectedState->w)
	{
		menu.w = menu.widgets[0]->selectedState->w + 20;
	}

	else
	{
		menu.w = menu.widgets[1]->selectedState->w + 20;
	}

	x = menu.widgets[2]->normalState->w + menu.widgets[3]->normalState->w + 20;

	menu.widgets[2]->x = (menu.w - x) / 2;
	menu.widgets[3]->x = menu.widgets[2]->x + menu.widgets[2]->selectedState->w + 20;

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	SDL_SetAlpha(menu.background, SDL_SRCALPHA|SDL_RLEACCEL, 196);

	free(buffer);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initYesNoMenu(char *text, void (*yes)(void), void (*no)(void))
{
	menu.action = &doMenu;

	yesAction = yes;
	noAction = no;

	if (menu.widgets != NULL)
	{
		freeYesNoMenu();
	}

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
	}

	if (menu.background != NULL)
	{
		SDL_FreeSurface(menu.background);

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
