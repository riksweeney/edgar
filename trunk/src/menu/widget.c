#include "../headers.h"

#include "../graphics/graphics.h"
#include "../graphics/font.h"

extern Game game;

Widget *createWidget(char *text, int *controlValue, int minValue, int maxValue, void (*clickAction)(void), int x, int y)
{
	Widget *w;

	w = (Widget *)malloc(sizeof(Widget));

	if (w == NULL)
	{
		printf("Failed to allocate %d bytes to create Widget %s\n", sizeof(Widget), text);

		exit(1);
	}

	w->normalState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);

	w->selectedState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 200, 0), 255, 255, 255, 0, 200, 0);

	w->value = controlValue;

	w->minValue = minValue;

	w->maxValue = maxValue;

	w->action = clickAction;

	w->x = x;

	w->y = y;

	return w;
}

void drawWidget(Widget *w, int selected)
{
	int x, y;

	x = w->x < 0 ? (SCREEN_WIDTH - w->normalState->w) / 2 : w->x;
	y = w->y < 0 ? (SCREEN_HEIGHT - w->normalState->h) / 2 : w->y;

	if (w != NULL)
	{
		drawImage(selected == TRUE ? w->selectedState : w->normalState, x, y, FALSE);
	}
}

void freeWidget(Widget *w)
{
	if (w != NULL)
	{
		if (w->normalState != NULL)
		{
			SDL_FreeSurface(w->normalState);

			w->normalState = NULL;
		}

		if (w->selectedState != NULL)
		{
			SDL_FreeSurface(w->selectedState);

			w->selectedState = NULL;
		}

		free(w);
	}
}
