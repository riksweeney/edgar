/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "headers.h"

#include "graphics/graphics.h"
#include "graphics/font.h"
#include "dialog.h"
#include "game.h"
#include "system/error.h"

extern Game game;

static DialogBox dialogBox;

void createDialogBoxFromScript(char *msg)
{
	char *text, *title, *savePtr;

	title = strtok_r(msg, " ", &savePtr);

	text = strtok_r(NULL, "\0", &savePtr);

	dialogBox.dialogSurface = createDialogBox(_(title), _(text));
}

void createAutoDialogBox(char *title, char *text, int thinkTime)
{
	dialogBox.dialogSurface = createDialogBox(title, text);
	dialogBox.thinkTime = thinkTime;
}

SDL_Surface *createDialogBox(char *title, char *msg)
{
	char *text, *token, word[MAX_VALUE_LENGTH], *savePtr, *titleText;
	int i, lines, w, h, maxWidth, lineBreak, *lineBreaks;
	SDL_Surface **surface, *tempSurface;
	SDL_Rect dest;

	savePtr = NULL;

	freeDialogBox();

	text = (char *)malloc(strlen(msg) + 1);

	if (text == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the Dialog Text", (int)strlen(msg) + 1);
	}
	
	STRNCPY(text, msg, strlen(msg) + 1);
	
	titleText = NULL;
	
	if (title != NULL)
	{
		titleText = (char *)malloc(strlen(title) + 1);

		if (titleText == NULL)
		{
			showErrorAndExit("Failed to allocate a whole %d bytes for the Dialog Text", (int)strlen(title) + 1);
		}

		STRNCPY(titleText, title, strlen(title) + 1);
	}

	token = strtok_r(text, " ", &savePtr);

	i = 0;

	while (token != NULL)
	{
		i++;

		token = strtok_r(NULL, " ", &savePtr);
	}

	lines = i;

	if (titleText != NULL)
	{
		token = titleText;
		
		while (*token != '\0')
		{
			if (*token == '_')
			{
				*token = ' ';
			}
			
			token++;
		}
		
		lines++;
	}

	surface = (SDL_Surface **)malloc(sizeof(SDL_Surface *) * lines);

	if (surface == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the Dialog Surfaces", (int)sizeof(SDL_Surface *) * lines);
	}

	lineBreaks = (int *)malloc(sizeof(int) * lines);

	if (lineBreaks == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the line breaks", (int)sizeof(int) * lines);
	}

	STRNCPY(text, msg, strlen(msg) + 1);

	token = strtok_r(text, " ", &savePtr);

	i = 0;

	maxWidth = w = h = 0;

	if (titleText != NULL)
	{
		surface[i] = generateTextSurface(titleText, game.font, 255, 255, 0, 0, 0, 0);

		h = surface[i]->h + 5;

		maxWidth = surface[i]->w;

		i++;
	}

	while (token != NULL)
	{
		lineBreak = FALSE;

		snprintf(word, sizeof(word), "%s ", token);
		
		if (strcmpignorecase(word, "[GAME_KILLS] ") == 0)
		{
			snprintf(word, sizeof(word), "%d ", game.kills);
		}
		
		if (strcmpignorecase(word, "[PLAY_TIME] ") == 0)
		{
			snprintf(word, sizeof(word), "%s ", getPlayTimeAsString());
		}

		else if (word[strlen(word) - 2] == '\n')
		{
			lineBreak = TRUE;

			word[strlen(word) - 2] = '\0';
		}

		surface[i] = generateTextSurface(word, game.font, 255, 255, 255, 0, 0, 0);

		lineBreaks[i] = lineBreak;

		if (h == 0 || (i == 1 && titleText != NULL))
		{
			h += surface[i]->h + 5;
		}

		if (w + surface[i]->w > MAX_DIALOG_WIDTH)
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		w += surface[i]->w;

		if (w > maxWidth)
		{
			maxWidth = w;
		}

		if (lineBreak == TRUE)
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		i++;

		token = strtok_r(NULL, " ", &savePtr);
	}

	h -= 5;

	tempSurface = createSurface(maxWidth, h);

	w = h = 0;

	for (i=0;i<lines;i++)
	{
		if (w + surface[i]->w > MAX_DIALOG_WIDTH || (titleText != NULL && i == 1))
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		dest.x = w;
		dest.y = h;
		dest.w = surface[i]->w;
		dest.h = surface[i]->h;

		SDL_BlitSurface(surface[i], NULL, tempSurface, &dest);

		w += surface[i]->w;

		SDL_FreeSurface(surface[i]);

		if (lineBreaks[i] == TRUE)
		{
			w = 0;

			h += surface[i]->h + 5;
		}
	}

	tempSurface = addBorder(tempSurface, 255, 255, 255, 0, 0, 0);

	free(surface);

	free(text);
	
	if (titleText != NULL)
	{
		free(titleText);
	}

	free(lineBreaks);

	return tempSurface;
}

void doDialogBox()
{
	if (dialogBox.thinkTime > 0)
	{
		dialogBox.thinkTime--;
		
		if (dialogBox.thinkTime == 0)
		{
			freeDialogBox();
		}
	}
}

void drawDialogBox()
{
	if (dialogBox.dialogSurface != NULL)
	{
		drawImage(dialogBox.dialogSurface, (SCREEN_WIDTH - dialogBox.dialogSurface->w) / 2, 50, FALSE, 255);
	}
}

void freeDialogBox()
{
	if (dialogBox.dialogSurface != NULL)
	{
		SDL_FreeSurface(dialogBox.dialogSurface);

		dialogBox.dialogSurface = NULL;
		
		dialogBox.thinkTime = 0;
	}
}
