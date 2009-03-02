#include "headers.h"

#include "random.h"
#include "resources.h"
#include "game.h"
#include "record.h"
#include "load_save.h"

extern Game game;

void init(char *title)
{
	int joysticks, buttons;
	
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

	/* Open a screen */

	game.screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_HWPALETTE|SDL_DOUBLEBUF|SDL_HWSURFACE);

	if (game.screen == NULL)
	{
		printf("Couldn't set screen mode to %d x %d: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());

		exit(1);
	}

	/* Set the audio rate to 22050, default mix, 2 channels and a 1024 byte buffer */

	game.audio = TRUE;

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
	{
		printf("Could not open audio: %s\n", Mix_GetError());

		game.audio = FALSE;
	}

	else
	{
		game.audioVolume = MIX_MAX_VOLUME;
	}
	
	joysticks = SDL_NumJoysticks();

	if (joysticks > 0)
	{
		printf("Found %d joysticks Opening Joystick #1: %s\n", joysticks, SDL_JoystickName(0));

		game.joystick = SDL_JoystickOpen(0);

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
	
	/* Set up the home directory */
	
	setupUserHomeDirectory();
}

void toggleFullScreen()
{
	SDL_WM_ToggleFullScreen(game.screen);
}

void cleanup()
{
	float fps;

	game.endTicks = SDL_GetTicks();

	fps = game.frames;

	fps /= (game.endTicks - game.startTicks);

	fps *= 1000;

	printf("%d frames in %ld milliseconds (%.2f fps)\n", game.frames, (game.endTicks - game.startTicks), fps);

	/* Stop the replay data */

	flushBuffer(game.gameType);

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

	/* Shut down SDL */

	SDL_Quit();
}
