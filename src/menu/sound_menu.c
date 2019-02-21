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
#include "../audio/music.h"
#include "../draw.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "label.h"
#include "options_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static char *getVolumePercent(int);
static void toggleSound(void);
static void changeVolume(int *, Widget *, int);
static void lowerSFXVolume(void);
static void raiseSFXVolume(void);
static void lowerMusicVolume(void);
static void raiseMusicVolume(void);
static void toggleQuality(void);
static char *getQuality(void);

void drawSoundMenu()
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
	char *text;
	int x, y, w, maxWidth, i;

	menu.widgetCount = 5;

	menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

	if (menu.widgets == NULL)
	{
		showErrorAndExit("Ran out of memory when creating Control Menu");
	}

	x = y = 0;

	menu.widgets[0] = createWidget(_("Sound"), &control.button[CONTROL_UP], &toggleSound, &toggleSound, &toggleSound, x, y, TRUE, 255, 255, 255);

	menu.widgets[0]->label = createLabel(game.audio == TRUE || game.audioDisabled == TRUE ? _("Yes") : _("No"), menu.widgets[0]->x + menu.widgets[0]->normalState->w + 10, y);

	menu.widgets[1] = createWidget(_("SFX Volume"), &game.sfxDefaultVolume, &lowerSFXVolume, &raiseSFXVolume, NULL, x, y, TRUE, 255, 255, 255);

	text = getVolumePercent(game.sfxDefaultVolume);

	menu.widgets[1]->label = createLabel(_(text), menu.widgets[1]->x + menu.widgets[1]->normalState->w + 10, y);

	free(text);

	menu.widgets[2] = createWidget(_("Music Volume"), &game.musicDefaultVolume, &lowerMusicVolume, &raiseMusicVolume, NULL, x, y, TRUE, 255, 255, 255);

	text = getVolumePercent(game.musicDefaultVolume);

	menu.widgets[2]->label = createLabel(_(text), menu.widgets[2]->x + menu.widgets[2]->normalState->w + 10, y);

	free(text);

	menu.widgets[3] = createWidget(_("Audio Quality"), &game.audioQuality, &toggleQuality, &toggleQuality, &toggleQuality, x, y, TRUE, 255, 255, 255);

	text = getQuality();

	menu.widgets[3]->label = createLabel(text, menu.widgets[3]->x + menu.widgets[3]->normalState->w + 10, y);

	free(text);

	menu.widgets[4] = createWidget(_("Back"), NULL, NULL, NULL, &showOptionsMenu, -1, y, TRUE, 255, 255, 255);

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

Menu *initSoundMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	menu.returnAction = &showOptionsMenu;

	return &menu;
}

void freeSoundMenu()
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

static void toggleSound()
{
	Widget *w = menu.widgets[menu.index];

	game.audio = game.audio == TRUE ? FALSE : TRUE;

	if (game.audio == FALSE)
	{
		game.audioDisabled = FALSE;

		freeMusic();

		Mix_CloseAudio();
	}

	else
	{
		if (initAudio() == TRUE)
		{
			playLoadedMusic();
		}

		else
		{
			game.audio = FALSE;
		}
	}

	updateLabelText(w->label, game.audio == TRUE ? _("Yes") : _("No"));
}

static void lowerSFXVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.sfxDefaultVolume, w, -1);
}

static void raiseSFXVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.sfxDefaultVolume, w, 1);
}

static void lowerMusicVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.musicDefaultVolume, w, -1);

	updateMusicVolume();
}

static void raiseMusicVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.musicDefaultVolume, w, 1);

	updateMusicVolume();
}

static void toggleQuality()
{
	char *text;
	Widget *w = menu.widgets[menu.index];

	game.audioQuality = game.audioQuality == 22050 ? 44100 : 22050;

	changeSoundQuality();

	text = getQuality();

	updateLabelText(w->label, text);

	free(text);
}

static void changeVolume(int *maxVolume, Widget *w, int adjustment)
{
	char *text;

	*maxVolume += adjustment;

	if (*maxVolume < 0)
	{
		*maxVolume = 0;
	}

	else if (*maxVolume > 10)
	{
		*maxVolume = 10;
	}

	text = getVolumePercent(*maxVolume);

	updateLabelText(w->label, text);

	free(text);
}

static char *getVolumePercent(int volume)
{
	char *text;

	text = malloc(3);

	if (text == NULL)
	{
		showErrorAndExit("Failed to allocate a whole 3 bytes for a Volume label");
	}

	snprintf(text, 3, "%d", volume);

	return text;
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}

static char *getQuality()
{
	char *text = malloc(10);

	if (text == NULL)
	{
		showErrorAndExit("Failed to allocate a whole 10 bytes for a Quality label");
	}

	snprintf(text, 10, "%d", game.audioQuality);

	return text;
}
