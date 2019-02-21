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

#include "headers.h"

#include "audio/audio.h"
#include "graphics/texture_cache.h"
#include "medal.h"
#include "system/load_save.h"
#include "system/pak.h"
#include "system/random.h"
#include "system/record.h"
#include "system/resources.h"

extern Game game;

void init(char *title, int joystickNum)
{
	int joysticks, buttons;
	long windowFlags, rendererFlags;

	windowFlags = 0;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	/* Set up the home directory */

	setupUserHomeDirectory();

	/* Initialise SDL Video and Audio */

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_AUDIO) < 0)
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

	/* Load the settings */

	loadConfig();

	if (game.fullscreen == TRUE)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	/* Open a screen */

	game.window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

	if (game.window == NULL)
	{
		printf("Couldn't set screen mode to %d x %d: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());

		exit(1);
	}

	if (game.audio == TRUE)
	{
		initAudio();
	}

	joysticks = game.disableJoystick == TRUE ? 0 : SDL_NumJoysticks();

	if (joysticks > 0 && (joystickNum < 0 || joystickNum >= joysticks))
	{
		printf("Joystick %d is not a valid joystick. Joysticks will be disabled\n", joystickNum);

		joysticks = 0;
	}

	buttons = 0;

	if (joysticks > 0)
	{
		printf("Found %d joysticks. Opening Joystick #%d\n", joysticks, joystickNum);

		game.joystick = SDL_JoystickOpen(joystickNum);

		buttons = SDL_JoystickNumButtons(game.joystick);

		printf("Joystick has %d buttons\n", buttons);

		printf("Joystick has %d axes\n", SDL_JoystickNumAxes(game.joystick));
	}

	/* Set the scaling hint */

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	/* Init the renderers */

	game.renderer = SDL_CreateRenderer(game.window, -1, rendererFlags);

	/* Enable blending */

	SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);

	/* Init SDL Image */
	IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);

	/* Hide the cursor */

	SDL_ShowCursor(0);

	/* Set the prandom seed */

	setSeed(time(NULL));

	/* Initalise the texture cache */
	initTextureCache();

	/* Init the PAK file */

	initPakFile();

	/* Init the medals */

	initMedals();
}

void toggleFullScreen()
{
	long fullScreen = SDL_GetWindowFlags(game.window) & SDL_WINDOW_FULLSCREEN;

	fullScreen ^= SDL_WINDOW_FULLSCREEN;

	SDL_SetWindowFullscreen(game.window, fullScreen);
}

void cleanup()
{
	float fps;

	game.endTicks = SDL_GetTicks();

	fps = game.frames;

	fps /= (game.endTicks - game.startTicks);

	fps *= 1000;

	#if DEV == 1
		printf("%d frames in %ld milliseconds (%.2f fps)\n", game.frames, (game.endTicks - game.startTicks), fps);
	#endif

	/* Stop the replay data */

	flushBuffer(game.gameType);

	/* Save the settings */

	if (game.status != IN_EDITOR)
	{
		saveConfig();
	}

	/* Save on exit */

	if (game.saveOnExit == TRUE)
	{
		saveGame(-1);
	}

	/* Free the medal queue */

	freeMedalQueue();

	/* Free the Resources */

	freeAllResources();

	/* Close SDL_TTF */

	TTF_Quit();

	/* Close the mixer */

	Mix_CloseAudio();

	/* Close the joystick */

	if (game.joystick != NULL)
	{
		SDL_JoystickClose(game.joystick);
	}

	printf("Exiting\n");

	/* Shut down SDL */

	SDL_DestroyRenderer(game.renderer);

	SDL_DestroyWindow(game.window);

	SDL_Quit();
}

void quitGame()
{
	exit(0);
}
