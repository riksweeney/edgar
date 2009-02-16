#include "headers.h"

#include "random.h"
#include "resources.h"
#include "game.h"
#include "record.h"

extern Game game;

void init(char *title)
{
	/* Initialise SDL Video and Audio */

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
	{
		printf("Could not initialize SDL: %s\n", SDL_GetError());

		exit(1);
	}

	/* Initialise SDL_TTF */

	if (TTF_Init() < 0)
	{
		printf("Couldn't initialize SDL TTF: %s\n", SDL_GetError());

		exit(1);
	}

	/* Open a screen */

	game.screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_HWPALETTE|SDL_DOUBLEBUF);

	if (game.screen == NULL)
	{
		printf("Couldn't set screen mode to %d x %d: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());

		exit(1);
	}

	/* Set the audio rate to 22050, 16 bit stereo, 2 channels and a 4096 byte buffer */

	game.audio = TRUE;

	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096) != 0)
	{
		printf("Could not open audio: %s\n", Mix_GetError());

		game.audio = FALSE;
	}

	else
	{
		game.audioVolume = MIX_MAX_VOLUME;
	}

	/* Set the screen title */

	SDL_WM_SetCaption(title, NULL);

	/* Hide the mouse cursor */

	SDL_ShowCursor(SDL_DISABLE);

	/* Set the prandom seed */

	setSeed(time(NULL));
}

void cleanup()
{
	printf("Exiting\n");

	/* Stop the replay data */

	flushBuffer(game.gameType);

	/* Free the Resources */

	freeAllResources();

	/* Close SDL_TTF */

	TTF_Quit();

	/* Close the mixer */

	Mix_CloseAudio();

	/* Shut down SDL */

	SDL_Quit();
}
