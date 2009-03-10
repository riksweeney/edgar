#include "headers.h"

#include "graphics.h"
#include "font.h"
#include "dialog.h"

extern Game game;

static SDL_Surface *dialogSurface;

void createDialogBoxFromScript(char *msg)
{
	char *token;
	
	token = strtok(msg, " ");
	
	token = strtok(NULL, " ");
	
	token = strtok(NULL, "\0");
	
	createDialogBox(token);
}

void createDialogBox(char *msg)
{
	char *text, *token, word[MAX_VALUE_LENGTH];
	int i, lines, w, h, maxWidth;
	SDL_Surface **surface, *temp;
	SDL_Rect dest;

	freeDialogBox();

	text = (char *)malloc(strlen(msg) + 1);

	if (text == NULL)
	{
		printf("Could not allocate a whole %d bytes for the Dialog Text\n", strlen(msg) + 1);

		exit(1);
	}

	STRNCPY(text, msg, strlen(msg) + 1);

	token = strtok(text, " ");

	i = 0;

	while (token != NULL)
	{
		i++;

		token = strtok(NULL, " ");
	}

	lines = i;

	printf("Creating %d surfaces for the dialog text\n", lines);

	surface = (SDL_Surface **)malloc(sizeof(SDL_Surface *) * lines);

	if (surface == NULL)
	{
		printf("Could not allocate a whole %d bytes for the Dialog Surfaces\n", sizeof(SDL_Surface *) * lines);

		exit(1);
	}

	STRNCPY(text, msg, strlen(msg) + 1);

	token = strtok(text, " ");

	i = 0;

	maxWidth = w = h = 0;

	while (token != NULL)
	{
		snprintf(word, sizeof(word), "%s ", token);
		
		surface[i] = generateTextSurface(word, game.font);
		
		if (h == 0)
		{
			h = surface[i]->h + 5;
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

		i++;

		token = strtok(NULL, " ");
	}

	h -= 5;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, maxWidth, h, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	dialogSurface = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	printf("Creating a dialog box of %d x %d\n", dialogSurface->w, dialogSurface->h);

	w = h = 0;

	for (i=0;i<lines;i++)
	{
		if (w + surface[i]->w > MAX_DIALOG_WIDTH)
		{
			w = 0;
			
			h += surface[i]->h + 5;
		}
		
		dest.x = w;
		dest.y = h;
		dest.w = surface[i]->w;
		dest.h = surface[i]->h;

		SDL_BlitSurface(surface[i], NULL, dialogSurface, &dest);

		printf("Blitted surface %d to %d %d\n", i, 0, h);
		
		w += surface[i]->w;

		SDL_FreeSurface(surface[i]);
	}

	free(surface);
	
	free(text);
}

void drawDialogBox()
{
	if (dialogSurface != NULL)
	{
		drawBorder((SCREEN_WIDTH - dialogSurface->w) / 2, 50, dialogSurface->w, dialogSurface->h, 255, 255, 255);

		drawImage(dialogSurface, (SCREEN_WIDTH - dialogSurface->w) / 2, 50, FALSE);
	}
}

void freeDialogBox()
{
	if (dialogSurface != NULL)
	{
		SDL_FreeSurface(dialogSurface);

		dialogSurface = NULL;
	}
}
