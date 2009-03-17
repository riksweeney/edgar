Widget *createWidget(char *text, int *controlValue, int minValue, int maxValue)
{
	Widget *w;

	w = (Widget *)malloc(sizeof(Widget));

	if (w == NULL)
	{
		printf("Failed to allocate %d bytes to create Widget %s\n", sizeof(Widget), text);

		exit(1);
	}

	w->normalState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255);

	w->selectedState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 200, 0), 255, 255, 255);

	w->value = controlValue;

	w->minValue = minValue;

	w->maxValue = maxValue;

	return w;
}

void freeWidget(Widget *w)
{
	if (w != NULL)
	{
		if (w->normalState != NULL)
		{
			SDL_FreeSurface(w->normalState);
		}

		if (w->selectedState != NULL)
		{
			SDL_FreeSurface(w->normalState);
		}

		free(w);
	}
}
