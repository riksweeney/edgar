#include "headers.h"

#include "record.h"
#include "init.h"

extern Input input;
extern Game game;

void getInput(int gameType)
{
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
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						exit(0);
					break;

					case SDLK_LEFT:
						input.left = TRUE;
					break;

					case SDLK_RIGHT:
						input.right = TRUE;
					break;

					case SDLK_UP:
						input.up = TRUE;
					break;

					case SDLK_DOWN:
						input.down = TRUE;
					break;

					case SDLK_SPACE:
						input.jump = TRUE;
						input.add = TRUE;
					break;

					case SDLK_PERIOD:
					case SDLK_EQUALS:
						input.next = TRUE;
					break;

					case SDLK_COMMA:
					case SDLK_MINUS:
						input.previous = TRUE;
					break;

					case SDLK_s:
						input.save = TRUE;
					break;

					case SDLK_l:
						input.load = TRUE;
					break;

					case SDLK_e:
						input.toggle = TRUE;
					break;

					case SDLK_d:
						input.drop = TRUE;
					break;

					case SDLK_a:
						input.activate = TRUE;
					break;

					case SDLK_x:
						input.cut = TRUE;
					break;

					case SDLK_RETURN:
						input.attack = TRUE;
					break;

					case SDLK_f:
						input.fly = TRUE;
					break;

					case SDLK_g:
						input.snap = TRUE;
					break;

					case SDLK_c:
						input.interact = TRUE;
						input.grabbing = TRUE;
					break;

					case SDLK_F12:
						if (game.gameType != RECORDING)
						{
							toggleFullScreen();
						}
					break;

					case SDLK_F10:
						if (game.gameType != RECORDING)
						{
							takeSingleScreenshot();
						}
					break;

					default:
					break;
				}
			break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
					case SDLK_LEFT:
						input.left = FALSE;
					break;

					case SDLK_RIGHT:
						input.right = FALSE;
					break;

					case SDLK_UP:
						input.up = FALSE;
					break;

					case SDLK_DOWN:
						input.down = FALSE;
					break;

					case SDLK_SPACE:
						input.jump = FALSE;
						input.add = FALSE;
					break;

					case SDLK_PERIOD:
					case SDLK_EQUALS:
						input.next = FALSE;
					break;

					case SDLK_COMMA:
					case SDLK_MINUS:
						input.previous = FALSE;
					break;

					case SDLK_s:
						input.save = FALSE;
					break;

					case SDLK_l:
						input.load = FALSE;
					break;

					case SDLK_e:
						input.toggle = FALSE;
					break;

					case SDLK_d:
						input.drop = FALSE;
					break;

					case SDLK_a:
						input.activate = FALSE;
					break;

					case SDLK_x:
						input.cut = FALSE;
					break;

					case SDLK_RETURN:
						input.attack = FALSE;
					break;

					case SDLK_f:
						input.fly = FALSE;
					break;

					case SDLK_g:
						input.snap = FALSE;
					break;

					case SDLK_c:
						input.interact = FALSE;
						input.grabbing = FALSE;
					break;
					
					default:
					break;
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
		}
	}

	/* Get the mouse coordinates */

	SDL_GetMouseState(&input.mouseX, &input.mouseY);

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
