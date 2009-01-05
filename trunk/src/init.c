#include "init.h"

extern void freeGame(void);
extern void freeRequiredResources(void);
extern void setSeed(long);

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
	
	if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 4096) != 0)
	{
		printf("Could not open audio: %s\n", Mix_GetError());
		
		game.audio = INACTIVE;
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
	/* Free the Resources */
	
	freeRequiredResources();
	
	/* Free Game data */
	
	freeGame();
	
	/* Close SDL_TTF */
	
	TTF_Quit();
	
	/* Close the mixer */
	
	Mix_CloseAudio();
	
	/* Shut down SDL */
	
	SDL_Quit();
}
