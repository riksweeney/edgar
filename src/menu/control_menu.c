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
#include "../input.h"
#include "../system/error.h"
#include "label.h"
#include "ok_menu.h"
#include "options_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static void redefineKey(void);
static void realignGrid(void);
static char *getDeadZoneValue(int);
static void raiseDeadZoneValue(void);
static void lowerDeadZoneValue(void);
static void showControlMenu(void);

void drawControlMenu()
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
			menu.index = 1;
		}

		playSound("sound/common/click");
	}

	else if (up == TRUE)
	{
		menu.index--;

		if (menu.index < 1)
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
			playSound("sound/common/click");

			w->clickAction();
		}
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
	char *text;
	int y;

	y = 0;

	menu.widgetCount = 16;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Control Menu");
	}

	menu.widgets[0] = createWidget(_("Use Up and Down to select. Enter to change."), NULL, NULL, NULL, &redefineKey, -1, y, FALSE, 255, 255, 255);

	menu.widgets[1] = createWidget(_("Up"), &control.button[CONTROL_UP], NULL, NULL, &redefineKey, 20, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_UP]);

	menu.widgets[1]->label = createLabel(text, menu.widgets[0]->x, y);

	menu.widgets[2] = createWidget(_("Down"), &control.button[CONTROL_DOWN], NULL, NULL, &redefineKey, 20, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_DOWN]);

	menu.widgets[2]->label = createLabel(text, menu.widgets[1]->x, y);

	menu.widgets[3] = createWidget(_("Left"), &control.button[CONTROL_LEFT], NULL, NULL, &redefineKey, 20, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_LEFT]);

	menu.widgets[3]->label = createLabel(text, menu.widgets[2]->x, y);

	menu.widgets[4] = createWidget(_("Right"), &control.button[CONTROL_RIGHT], NULL, NULL, &redefineKey, 20, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_RIGHT]);

	menu.widgets[4]->label = createLabel(text, menu.widgets[3]->x, y);

	menu.widgets[5] = createWidget(_("Attack"), &control.button[CONTROL_ATTACK], NULL, NULL, &redefineKey, 20, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_ATTACK]);

	menu.widgets[5]->label = createLabel(text, menu.widgets[4]->x, y);

	menu.widgets[6] = createWidget(_("Block"), &control.button[CONTROL_BLOCK], NULL, NULL, &redefineKey, 20, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_BLOCK]);

	menu.widgets[6]->label = createLabel(text, menu.widgets[5]->x, y);

	menu.widgets[7] = createWidget(_("Jump"), &control.button[CONTROL_JUMP], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_JUMP]);

	menu.widgets[7]->label = createLabel(text, menu.widgets[6]->x, y);

	menu.widgets[8] = createWidget(_("Interact"), &control.button[CONTROL_INTERACT], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_INTERACT]);

	menu.widgets[8]->label = createLabel(text, menu.widgets[7]->x, y);

	menu.widgets[9] = createWidget(_("Use"), &control.button[CONTROL_ACTIVATE], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_ACTIVATE]);

	menu.widgets[9]->label = createLabel(text, menu.widgets[8]->x, y);

	menu.widgets[10] = createWidget(_("Previous Item"), &control.button[CONTROL_PREVIOUS], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_PREVIOUS]);

	menu.widgets[10]->label = createLabel(text, menu.widgets[9]->x, y);

	menu.widgets[11] = createWidget(_("Next Item"), &control.button[CONTROL_NEXT], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_NEXT]);

	menu.widgets[11]->label = createLabel(text, menu.widgets[10]->x, y);

	menu.widgets[12] = createWidget(_("Inventory"), &control.button[CONTROL_INVENTORY], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_INVENTORY]);

	menu.widgets[12]->label = createLabel(text, menu.widgets[11]->x, y);

	menu.widgets[13] = createWidget(_("Pause"), &control.button[CONTROL_PAUSE], NULL, NULL, &redefineKey, 40, y, TRUE, 255, 255, 255);

	text = getKeyValue(control.button[CONTROL_PAUSE]);

	menu.widgets[13]->label = createLabel(text, menu.widgets[12]->x, y);

	menu.widgets[14] = createWidget(_("Dead Zone"), &control.deadZone, &lowerDeadZoneValue, &raiseDeadZoneValue, NULL, 40, y, TRUE, 255, 255, 255);

	text = getDeadZoneValue(control.deadZone);

	menu.widgets[14]->label = createLabel(text, menu.widgets[13]->x + menu.widgets[13]->normalState->w + 10, y);

	menu.widgets[15] = createWidget(_("Back"), NULL, NULL, NULL, &showOptionsMenu, -1, y, TRUE, 255, 255, 255);

	realignGrid();
}

Menu *initControlMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	menu.returnAction = &showOptionsMenu;

	menu.index = 1;

	return &menu;
}

static void realignGrid()
{
	int i, y, w, maxWidth1, maxWidth2, colWidth1, colWidth2;

	y = BUTTON_PADDING + BORDER_PADDING;

	maxWidth1 = maxWidth2 = w = 0;

	colWidth1 = colWidth2 = 0;

	menu.widgets[0]->y = y = BUTTON_PADDING + BORDER_PADDING;

	colWidth2 = menu.widgets[0]->selectedState->w;

	y += menu.widgets[0]->selectedState->h + BUTTON_PADDING;

	for (i=1;i<8;i++)
	{
		if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth1)
		{
			maxWidth1 = menu.widgets[i]->normalState->w;
		}
	}

	for (i=1;i<8;i++)
	{
		menu.widgets[i]->y = y;

		if (menu.widgets[i]->x != -1)
		{
			menu.widgets[i]->x = BUTTON_PADDING + BORDER_PADDING;
		}

		if (menu.widgets[i]->label != NULL)
		{
			menu.widgets[i]->label->y = y;

			menu.widgets[i]->label->x = menu.widgets[i]->x + maxWidth1 + 10;

			if (menu.widgets[i]->label->x + menu.widgets[i]->label->text->w > colWidth1)
			{
				colWidth1 = menu.widgets[i]->label->x + menu.widgets[i]->label->text->w;
			}
		}

		else
		{
			if (menu.widgets[i]->x + menu.widgets[i]->selectedState->w > colWidth1)
			{
				colWidth1 = menu.widgets[i]->x + menu.widgets[i]->selectedState->w;
			}
		}

		y += menu.widgets[i]->selectedState->h + BUTTON_PADDING;
	}

	y = menu.widgets[1]->y;

	for (i=8;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth2)
		{
			maxWidth2 = menu.widgets[i]->normalState->w;
		}
	}

	for (i=8;i<menu.widgetCount;i++)
	{
		menu.widgets[i]->y = y;

		if (menu.widgets[i]->x != -1)
		{
			menu.widgets[i]->x = colWidth1 + BUTTON_PADDING + BORDER_PADDING;
		}

		if (menu.widgets[i]->label != NULL)
		{
			menu.widgets[i]->label->y = y;

			menu.widgets[i]->label->x = menu.widgets[i]->x + maxWidth2 + 10;

			if (menu.widgets[i]->label->x + menu.widgets[i]->label->text->w > colWidth2)
			{
				colWidth2 = menu.widgets[i]->label->x + menu.widgets[i]->label->text->w;
			}
		}

		else
		{
			if (menu.widgets[i]->x + menu.widgets[i]->selectedState->w > colWidth2)
			{
				colWidth2 = menu.widgets[i]->x + menu.widgets[i]->selectedState->w;
			}
		}

		y += menu.widgets[i]->selectedState->h + BUTTON_PADDING;
	}

	w = colWidth2 + BUTTON_PADDING;

	if (menu.w != w)
	{
		if (menu.background != NULL)
		{
			destroyTexture(menu.background);

			menu.background = NULL;
		}

		menu.w = w;
		menu.h = y - BORDER_PADDING;

		menu.background = addBorder(createSurface(menu.w, menu.h, FALSE), 255, 255, 255, 0, 0, 0);

		menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
		menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
	}
}

void freeControlMenu()
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

static void redefineKey()
{
	int i, key, oldKey;
	char *text;
	Widget *w = menu.widgets[menu.index];

	key = -2;

	updateLabelText(w->label, "?");

	oldKey = (*w->value);

	flushInputs();

	while (key == -2)
	{
		key = getSingleInput();

		draw();
	}

	if (key != -1)
	{
		(*w->value) = key;
	}

	else
	{
		key = oldKey;
	}

	text = getKeyValue(key);

	updateLabelText(w->label, text);

	for (i=0;i<menu.widgetCount;i++)
	{
		if (i == menu.index)
		{
			continue;
		}

		if (menu.widgets[i]->value != NULL && *menu.widgets[i]->value == key)
		{
			*menu.widgets[i]->value = -1;

			updateLabelText(menu.widgets[i]->label, "?");
		}
	}

	realignGrid();
}

static void lowerDeadZoneValue()
{
	char *text;
	Widget *w = menu.widgets[menu.index];

	control.deadZone -= 1000;

	if (control.deadZone < 0)
	{
		control.deadZone = 0;
	}

	text = getDeadZoneValue(control.deadZone);

	updateLabelText(w->label, text);

	free(text);
}

static void raiseDeadZoneValue()
{
	char *text;
	Widget *w = menu.widgets[menu.index];

	control.deadZone += 1000;

	if (control.deadZone > 32000)
	{
		control.deadZone = 32000;
	}

	text = getDeadZoneValue(control.deadZone);

	updateLabelText(w->label, text);

	free(text);
}

static void showOptionsMenu()
{
	int i, j, valid;

	valid = TRUE;

	for (i=0;i<menu.widgetCount;i++)
	{
		if (menu.widgets[i]->value == NULL)
		{
			continue;
		}

		if (*menu.widgets[i]->value == -1)
		{
			valid = FALSE;

			break;
		}

		for (j=0;j<menu.widgetCount;j++)
		{
			if (i == j || menu.widgets[j]->value == NULL)
			{
				continue;
			}

			if (*menu.widgets[i]->value == *menu.widgets[j]->value)
			{
				valid = FALSE;
			}
		}
	}

	if (valid == TRUE)
	{
		game.menu = initOptionsMenu();

		game.drawMenu = &drawOptionsMenu;
	}

	else
	{
		game.menu = initOKMenu(_("Please configure all controls"), &showControlMenu);

		game.drawMenu = &drawOKMenu;
	}
}

static void showControlMenu()
{
	game.menu = initControlMenu();

	game.drawMenu = &drawControlMenu;
}

static char *getDeadZoneValue(int value)
{
	char *text;

	text = malloc(10);

	if (text == NULL)
	{
		showErrorAndExit("Failed to allocate a whole 10 bytes for the dead zone label");
	}

	snprintf(text, 10, "%d", value);

	return text;
}
