/*
Copyright (C) 2009-2012 Parallel Realities

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
	long flags;

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

	flags = SDL_HWPALETTE|SDL_DOUBLEBUF|SDL_HWSURFACE;

	/* Load the settings */

	loadConfig();

	if (game.fullscreen == TRUE)
	{
		flags |= SDL_FULLSCREEN;
	}

	/* Open a screen */

	game.screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, flags);

	if (game.screen == NULL)
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
		printf("Found %d joysticks. Opening Joystick #%d: %s\n", joysticks, joystickNum, SDL_JoystickName(joystickNum));

		game.joystick = SDL_JoystickOpen(joystickNum);

		buttons = SDL_JoystickNumButtons(game.joystick);

		printf("Joystick has %d buttons\n", buttons);

		printf("Joystick has %d axes\n", SDL_JoystickNumAxes(game.joystick));
	}

	/* Set the screen title */

	SDL_WM_SetCaption(title, NULL);

	/* Hide the mouse cursor */

	SDL_ShowCursor(SDL_DISABLE);

	/* Set the prandom seed */

	setSeed(time(NULL));

	/* Init the PAK file */

	initPakFile();

	/* Init the medals */

	initMedals();
}

void toggleFullScreen()
{
	long flags = game.screen->flags;

	flags ^= SDL_FULLSCREEN;

	game.screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, flags);
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

	SDL_Quit();
}

void quitGame()
{
	exit(0);
}
