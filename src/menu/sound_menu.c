/*
Copyright (C) 2009-2011 Parallel Realities

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

#include "../audio/audio.h"
#include "../audio/music.h"
#include "../draw.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/pak.h"
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
static void realignGrid(void);
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
	char *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2, *text;
	unsigned char *buffer;
	int x, y, i;

	savePtr1 = NULL;

	i = 0;

	buffer = loadFileFromPak("data/menu/sound_menu.dat");

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

			menu.widgetCount = atoi(token);

			menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				showErrorAndExit("Ran out of memory when creating Control Menu");
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "SOUND") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), &control.button[CONTROL_UP], &toggleSound, &toggleSound, &toggleSound, x, y, TRUE, 255, 255, 255);

					menu.widgets[i]->label = createLabel(game.audio == TRUE || game.audioDisabled == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);
				}

				else if (strcmpignorecase(menuID, "SFX_VOLUME") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), &game.sfxDefaultVolume, &lowerSFXVolume, &raiseSFXVolume, NULL, x, y, TRUE, 255, 255, 255);

					text = getVolumePercent(game.sfxDefaultVolume);

					menu.widgets[i]->label = createLabel(_(text), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					free(text);
				}

				else if (strcmpignorecase(menuID, "MUSIC_VOLUME") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), &game.musicDefaultVolume, &lowerMusicVolume, &raiseMusicVolume, NULL, x, y, TRUE, 255, 255, 255);

					text = getVolumePercent(game.musicDefaultVolume);

					menu.widgets[i]->label = createLabel(_(text), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					free(text);
				}

				else if (strcmpignorecase(menuID, "BUFFER") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), &game.audioQuality, &toggleQuality, &toggleQuality, &toggleQuality, x, y, TRUE, 255, 255, 255);

					text = getQuality();

					menu.widgets[i]->label = createLabel(text, menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					free(text);
				}

				else if (strcmpignorecase(menuID, "MENU_BACK") == 0)
				{
					menu.widgets[i] = createWidget(_(menuName), NULL, NULL, NULL, &showOptionsMenu, x, y, TRUE, 255, 255, 255);
				}

				else
				{
					showErrorAndExit("Unknown widget %s", menuID);
				}

				i++;
			}

			else
			{
				showErrorAndExit("Widget Count must be defined!");
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w <= 0 || menu.h <= 0)
	{
		showErrorAndExit("Menu dimensions must be greater than 0");
	}

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	free(buffer);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;

	realignGrid();
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

static void realignGrid()
{
	int i, maxWidth = 0;

	if (menu.widgets != NULL)
	{
		for (i=0;i<menu.widgetCount;i++)
		{
			if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth)
			{
				maxWidth = menu.widgets[i]->normalState->w;
			}
		}

		for (i=0;i<menu.widgetCount;i++)
		{
			if (menu.widgets[i]->label != NULL)
			{
				menu.widgets[i]->label->x = menu.widgets[i]->x + maxWidth + 10;
			}
		}
	}
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
	}

	if (menu.background != NULL)
	{
		SDL_FreeSurface(menu.background);

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
	
	pauseMusic(FALSE);

	changeVolume(&game.musicDefaultVolume, w, -1);
}

static void raiseMusicVolume()
{
	Widget *w = menu.widgets[menu.index];
	
	pauseMusic(FALSE);

	changeVolume(&game.musicDefaultVolume, w, 1);
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
		showErrorAndExit("Failed to allocate a whole 3 bytes for a volume label");
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
