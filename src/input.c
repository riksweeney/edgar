#include "headers.h"

#include "system/record.h"
#include "init.h"
#include "game.h"

extern Input input;
extern Game game;

static Control control;

void getInput(int gameType)
{
	int key;
	SDL_Event event;

	/* Loop through waiting messages and process them */

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
			break;

			case SDL_KEYDOWN:
				key = event.key.keysym.sym;

				if (key == SDLK_ESCAPE)
				{
					exit(0);
				}

				else if (key == control.button[CONTROL_LEFT])
				{
					input.left = TRUE;
				}

				else if (key == control.button[CONTROL_RIGHT])
				{
					input.right = TRUE;
				}

				else if (key == control.button[CONTROL_UP])
				{
					input.up = TRUE;
				}

				else if (key == control.button[CONTROL_DOWN])
				{
					input.down = TRUE;
				}

				else if (key == control.button[CONTROL_JUMP])
				{
					input.jump = TRUE;
				}

				else if (key == control.button[CONTROL_NEXT])
				{
					input.next = TRUE;
				}

				else if (key == control.button[CONTROL_PREVIOUS])
				{
					input.previous = TRUE;
				}

				else if (key == control.button[CONTROL_SAVE])
				{
					input.save = TRUE;
				}

				else if (key == control.button[CONTROL_LOAD])
				{
					input.load = TRUE;
				}

				else if (key == control.button[CONTROL_TOGGLE])
				{
					input.toggle = TRUE;
				}

				else if (key == control.button[CONTROL_ACTIVATE])
				{
					input.activate = TRUE;
				}

				else if (key == control.button[CONTROL_CUT])
				{
					input.cut = TRUE;
				}

				else if (key == control.button[CONTROL_ATTACK])
				{
					input.attack = TRUE;
				}

				else if (key == control.button[CONTROL_FLY])
				{
					input.fly = TRUE;
				}

				else if (key == control.button[CONTROL_SNAP])
				{
					input.snap = TRUE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = TRUE;
					input.grabbing = TRUE;
				}

				else if (key == SDLK_F12 && game.gameType != RECORDING)
				{
					toggleFullScreen();
				}

				else if (key == SDLK_F10 && game.gameType != RECORDING)
				{
					takeSingleScreenshot();
				}
				
				else if (key == control.button[CONTROL_PAUSE])
				{
					pauseGame();
				}
			break;

			case SDL_KEYUP:
				key = event.key.keysym.sym;

				if (key == control.button[CONTROL_LEFT])
				{
					input.left = FALSE;
				}

				else if (key == control.button[CONTROL_RIGHT])
				{
					input.right = FALSE;
				}

				else if (key == control.button[CONTROL_UP])
				{
					input.up = FALSE;
				}

				else if (key == control.button[CONTROL_DOWN])
				{
					input.down = FALSE;
				}

				else if (key == control.button[CONTROL_JUMP])
				{
					input.jump = FALSE;
				}

				else if (key == control.button[CONTROL_NEXT])
				{
					input.next = FALSE;
				}

				else if (key == control.button[CONTROL_PREVIOUS])
				{
					input.previous = FALSE;
				}

				else if (key == control.button[CONTROL_SAVE])
				{
					input.save = FALSE;
				}

				else if (key == control.button[CONTROL_LOAD])
				{
					input.load = FALSE;
				}

				else if (key == control.button[CONTROL_TOGGLE])
				{
					input.toggle = FALSE;
				}

				else if (key == control.button[CONTROL_ACTIVATE])
				{
					input.activate = FALSE;
				}

				else if (key == control.button[CONTROL_CUT])
				{
					input.cut = FALSE;
				}

				else if (key == control.button[CONTROL_ATTACK])
				{
					input.attack = FALSE;
				}

				else if (key == control.button[CONTROL_FLY])
				{
					input.fly = FALSE;
				}

				else if (key == control.button[CONTROL_SNAP])
				{
					input.snap = FALSE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = FALSE;
					input.grabbing = FALSE;
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
				switch(event.button.button)
				{
					case SDL_BUTTON_LEFT:
						input.add = TRUE;
					break;

					case SDL_BUTTON_RIGHT:
						input.remove = TRUE;
					break;

					default:
					break;
				}
			break;

			case SDL_MOUSEBUTTONUP:
				switch(event.button.button)
				{
					case SDL_BUTTON_LEFT:
						input.add = FALSE;
					break;

					case SDL_BUTTON_RIGHT:
						input.remove = FALSE;
					break;

					default:
					break;
				}
			break;
			
			case SDL_JOYBUTTONDOWN:
				key = event.jbutton.button;
				
				if (key == control.button[CONTROL_LEFT])
				{
					input.left = TRUE;
				}

				else if (key == control.button[CONTROL_RIGHT])
				{
					input.right = TRUE;
				}

				else if (key == control.button[CONTROL_UP])
				{
					input.up = TRUE;
				}

				else if (key == control.button[CONTROL_DOWN])
				{
					input.down = TRUE;
				}

				else if (key == control.button[CONTROL_JUMP])
				{
					input.jump = TRUE;
				}

				else if (key == control.button[CONTROL_NEXT])
				{
					input.next = TRUE;
				}

				else if (key == control.button[CONTROL_PREVIOUS])
				{
					input.previous = TRUE;
				}

				else if (key == control.button[CONTROL_TOGGLE])
				{
					input.toggle = TRUE;
				}

				else if (key == control.button[CONTROL_ACTIVATE])
				{
					input.activate = TRUE;
				}

				else if (key == control.button[CONTROL_ATTACK])
				{
					input.attack = TRUE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = TRUE;
					input.grabbing = TRUE;
				}
				
				else if (key == control.button[CONTROL_PAUSE])
				{
					pauseGame();
				}
			break;
			
			case SDL_JOYBUTTONUP:
				key = event.jbutton.button;
				
				if (key == control.button[CONTROL_LEFT])
				{
					input.left = FALSE;
				}

				else if (key == control.button[CONTROL_RIGHT])
				{
					input.right = FALSE;
				}

				else if (key == control.button[CONTROL_UP])
				{
					input.up = FALSE;
				}

				else if (key == control.button[CONTROL_DOWN])
				{
					input.down = FALSE;
				}

				else if (key == control.button[CONTROL_JUMP])
				{
					input.jump = FALSE;
				}

				else if (key == control.button[CONTROL_NEXT])
				{
					input.next = FALSE;
				}

				else if (key == control.button[CONTROL_PREVIOUS])
				{
					input.previous = FALSE;
				}

				else if (key == control.button[CONTROL_ACTIVATE])
				{
					input.activate = FALSE;
				}

				else if (key == control.button[CONTROL_ATTACK])
				{
					input.attack = FALSE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = FALSE;
					input.grabbing = FALSE;
				}
			break;
			
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0)
				{
					if (event.jaxis.value < -3200)
					{
						input.left = TRUE;
					}
					
					else if (event.jaxis.value > 3200)
					{
						input.right = TRUE;
					}
					
					else
					{
						input.left = FALSE;
						input.right = FALSE;
					}
				}

				if (event.jaxis.axis == 1)
				{
					if (event.jaxis.value < -3200)
					{
						input.up = TRUE;
					}
					
					else if (event.jaxis.value > 3200)
					{
						input.down = TRUE;
					}
					
					else
					{
						input.up = FALSE;
						input.down = FALSE;
					}
				}
			break;
		}
	}

	/* Get the mouse coordinates */

	SDL_GetMouseState(&input.mouseX, &input.mouseY);
	
	if (game.paused == FALSE)
	{
		switch (gameType)
		{
			case RECORDING:
				putBuffer(input);
			break;
	
			case REPLAYING:
				input = getBuffer();
			break;
	
			default:
			break;
		}
	}
}

void setDefaultControls(int editor)
{
	if (game.joystick != NULL && editor == FALSE)
	{
		control.button[CONTROL_LEFT] = -1;
		control.button[CONTROL_RIGHT] = -1;
		control.button[CONTROL_UP] = -1;
		control.button[CONTROL_DOWN] = -1;
		control.button[CONTROL_JUMP] = 1;
		control.button[CONTROL_ADD] = -1;
		control.button[CONTROL_NEXT] = 7;
		control.button[CONTROL_PREVIOUS] = 6;
		control.button[CONTROL_SAVE] = -1;
		control.button[CONTROL_LOAD] = -1;
		control.button[CONTROL_TOGGLE] = -1;
		control.button[CONTROL_ACTIVATE] = 0;
		control.button[CONTROL_CUT] = -1;
		control.button[CONTROL_ATTACK] = 2;
		control.button[CONTROL_FLY] = -1;
		control.button[CONTROL_SNAP] = -1;
		control.button[CONTROL_INTERACT] = 3;
		control.button[CONTROL_PAUSE] = 9;
	}
	
	else
	{
		control.button[CONTROL_LEFT] = SDLK_LEFT;
		control.button[CONTROL_RIGHT] = SDLK_RIGHT;
		control.button[CONTROL_UP] = SDLK_UP;
		control.button[CONTROL_DOWN] = SDLK_DOWN;
		control.button[CONTROL_JUMP] = SDLK_SPACE;
		control.button[CONTROL_ADD] = SDLK_LEFT;
		control.button[CONTROL_NEXT] = SDLK_PERIOD;
		control.button[CONTROL_PREVIOUS] = SDLK_COMMA;
		control.button[CONTROL_SAVE] = SDLK_s;
		control.button[CONTROL_LOAD] = SDLK_l;
		control.button[CONTROL_TOGGLE] = SDLK_e;
		control.button[CONTROL_ACTIVATE] = SDLK_a;
		control.button[CONTROL_CUT] = SDLK_x;
		control.button[CONTROL_ATTACK] = SDLK_RETURN;
		control.button[CONTROL_FLY] = SDLK_d;
		control.button[CONTROL_SNAP] = SDLK_g;
		control.button[CONTROL_INTERACT] = SDLK_c;
		control.button[CONTROL_PAUSE] = SDLK_p;
	}
}
