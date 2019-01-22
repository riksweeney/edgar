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

#include "credits.h"
#include "game.h"
#include "graphics/graphics.h"
#include "init.h"
#include "system/record.h"

extern Input input, menuInput;
extern Game game;
extern Control control;
extern Entity player;

static char *getJoystickButton(int);

void getInput(int gameType)
{
	int key, button;
	SDL_Event event;

	/* Loop through waiting messages and process them */

	while (SDL_PollEvent(&event))
	{
		key = button = -1;

		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
			break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_FOCUS_LOST:
						focusLost();
					break;
				}
			break;

			case SDL_KEYDOWN:
				key = event.key.keysym.sym;

				input.lastPressedKey = key;

				if (game.status == IN_ERROR)
				{
					if (key == SDLK_ESCAPE)
					{
						exit(0);
					}
				}

				else if (game.status == IN_TITLE)
				{
					pauseGame();

					return;
				}

				else if (game.status == IN_CREDITS)
				{
					if (key == SDLK_ESCAPE || key == control.button[CONTROL_PAUSE] || key == control.button[CONTROL_ATTACK])
					{
						fadeCredits();
					}
				}

				else if (key == SDLK_ESCAPE || key == control.button[CONTROL_PAUSE])
				{
					pauseGame();
				}

				else if (key == control.button[CONTROL_INVENTORY])
				{
					if (!(player.flags & HELPLESS) && player.action == NULL)
					{
						pauseGameInventory();
					}

					input.inventory = TRUE;
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

				else if (key == control.button[CONTROL_SNAP])
				{
					input.snap = TRUE;
				}

				else if (key == control.button[CONTROL_BLOCK])
				{
					input.block = TRUE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = TRUE;
					input.grabbing = TRUE;
				}

				else if (key == FULLSCREEN_KEY && game.gameType != RECORDING)
				{
					toggleFullScreen();
				}

				else if (key == SCREENSHOT_KEY && game.gameType != RECORDING)
				{
					takeSingleScreenshot("edgar.png");
				}

				if (key == SDLK_UP)
				{
					menuInput.up = TRUE;
				}

				else if (key == SDLK_DOWN)
				{
					menuInput.down = TRUE;
				}

				else if (key == SDLK_RIGHT)
				{
					menuInput.right = TRUE;
				}

				else if (key == SDLK_LEFT)
				{
					menuInput.left = TRUE;
				}

				else if (key == SDLK_RETURN)
				{
					menuInput.attack = TRUE;
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

				else if (key == control.button[CONTROL_SNAP])
				{
					input.snap = FALSE;
				}

				else if (key == control.button[CONTROL_BLOCK])
				{
					input.block = FALSE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = FALSE;
					input.grabbing = FALSE;
				}

				if (key == SDLK_UP)
				{
					menuInput.up = FALSE;
				}

				else if (key == SDLK_DOWN)
				{
					menuInput.down = FALSE;
				}

				else if (key == SDLK_RIGHT)
				{
					menuInput.right = FALSE;
				}

				else if (key == SDLK_LEFT)
				{
					menuInput.left = FALSE;
				}

				else if (key == SDLK_RETURN)
				{
					menuInput.attack = FALSE;
				}

				else if (key == control.button[CONTROL_INVENTORY])
				{
					input.inventory = FALSE;
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

				key = -(key + 1000);

				if (game.status == IN_CREDITS)
				{
					if (key == control.button[CONTROL_PAUSE] || key == control.button[CONTROL_ATTACK])
					{
						fadeCredits();
					}
				}

				else if (game.status == IN_TITLE)
				{
					pauseGame();

					return;
				}

				else if (key == control.button[CONTROL_PAUSE])
				{
					pauseGame();
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

				else if (key == control.button[CONTROL_BLOCK])
				{
					input.block = TRUE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = TRUE;
					input.grabbing = TRUE;
				}

				else if (key == control.button[CONTROL_INVENTORY])
				{
					if (!(player.flags & HELPLESS) && player.action == NULL)
					{
						pauseGameInventory();
					}

					input.inventory = TRUE;
				}
			break;

			case SDL_JOYBUTTONUP:
				key = event.jbutton.button;

				key = -(key + 1000);

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

				else if (key == control.button[CONTROL_BLOCK])
				{
					input.block = FALSE;
				}

				else if (key == control.button[CONTROL_INTERACT])
				{
					input.interact = FALSE;
					input.grabbing = FALSE;
				}

				else if (key == control.button[CONTROL_INVENTORY])
				{
					input.inventory = FALSE;
				}
			break;

			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0)
				{
					if (event.jaxis.value < -control.deadZone)
					{
						if (input.xAxisMoved == FALSE || input.right == TRUE)
						{
							input.left = TRUE;
							input.right = FALSE;
							input.xAxisMoved = TRUE;
						}
					}

					else if (event.jaxis.value > control.deadZone)
					{
						if (input.xAxisMoved == FALSE || input.left == TRUE)
						{
							input.left = FALSE;
							input.right = TRUE;
							input.xAxisMoved = TRUE;
						}
					}

					else
					{
						input.left = FALSE;
						input.right = FALSE;
						input.xAxisMoved = FALSE;
					}
				}

				if (event.jaxis.axis == 1)
				{
					if (event.jaxis.value < -control.deadZone)
					{
						if (input.yAxisMoved == FALSE || input.down == TRUE)
						{
							input.up = TRUE;
							input.down = FALSE;
							input.yAxisMoved = TRUE;
						}
					}

					else if (event.jaxis.value > control.deadZone)
					{
						if (input.yAxisMoved == FALSE || input.up == TRUE)
						{
							input.up = FALSE;
							input.down = TRUE;
							input.yAxisMoved = TRUE;
						}
					}

					else
					{
						input.up = FALSE;
						input.down = FALSE;
						input.yAxisMoved = FALSE;
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

				if (input.inventory == TRUE)
				{
					if (!(player.flags & HELPLESS) && player.action == NULL)
					{
						pauseGameInventory();
					}
				}
			break;

			default:
			break;
		}
	}

	input.inventory = FALSE;
}

void resetControls(int editor)
{
	control.button[CONTROL_LEFT] = -1;
	control.button[CONTROL_RIGHT] = -1;
	control.button[CONTROL_UP] = -1;
	control.button[CONTROL_DOWN] = -1;
	control.button[CONTROL_JUMP] = 1;
	control.button[CONTROL_ADD] = -1;
	control.button[CONTROL_NEXT] = -1;
	control.button[CONTROL_PREVIOUS] = -1;
	control.button[CONTROL_SAVE] = -1;
	control.button[CONTROL_LOAD] = -1;
	control.button[CONTROL_TOGGLE] = -1;
	control.button[CONTROL_ACTIVATE] = -1;
	control.button[CONTROL_CUT] = -1;
	control.button[CONTROL_ATTACK] = -1;
	control.button[CONTROL_SNAP] = -1;
	control.button[CONTROL_INTERACT] = -1;
	control.button[CONTROL_PAUSE] = -1;
	control.button[CONTROL_BLOCK] = -1;

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
		control.button[CONTROL_SNAP] = -1;
		control.button[CONTROL_INTERACT] = 3;
		control.button[CONTROL_PAUSE] = 9;
		control.button[CONTROL_BLOCK] = -1;
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
		control.button[CONTROL_ATTACK] = SDLK_LCTRL;
		control.button[CONTROL_INVENTORY] = SDLK_z;
		control.button[CONTROL_ACTIVATE] = SDLK_a;
		control.button[CONTROL_INTERACT] = SDLK_c;
		control.button[CONTROL_PAUSE] = SDLK_p;
		control.button[CONTROL_BLOCK] = SDLK_LALT;

		if (editor == TRUE)
		{
			control.button[CONTROL_SAVE] = SDLK_s;
			control.button[CONTROL_LOAD] = SDLK_l;
			control.button[CONTROL_TOGGLE] = SDLK_e;
			control.button[CONTROL_CUT] = SDLK_x;
			control.button[CONTROL_SNAP] = SDLK_g;
		}
	}

	control.deadZone = 6000;
}

void flushInputs()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {}
}

int getSingleInput()
{
	int key;
	SDL_Event event;

	key = -2;

	if (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
			break;

			case SDL_KEYDOWN:
				key = event.key.keysym.sym;
			break;

			case SDL_JOYBUTTONDOWN:
				key = event.jbutton.button;

				key = -(key + 1000);
			break;
		}
	}

	return (key == SDLK_ESCAPE || key == SCREENSHOT_KEY || key == FULLSCREEN_KEY) ? -1 : key;
}

void writeControlsToFile(FILE *fp)
{
	fprintf(fp, "CONTROLS\n");
	fprintf(fp, "LEFT %d\n", control.button[CONTROL_LEFT]);
	fprintf(fp, "RIGHT %d\n", control.button[CONTROL_RIGHT]);
	fprintf(fp, "UP %d\n", control.button[CONTROL_UP]);
	fprintf(fp, "DOWN %d\n", control.button[CONTROL_DOWN]);
	fprintf(fp, "JUMP %d\n", control.button[CONTROL_JUMP]);
	fprintf(fp, "ATTACK %d\n", control.button[CONTROL_ATTACK]);
	fprintf(fp, "BLOCK %d\n", control.button[CONTROL_BLOCK]);
	fprintf(fp, "INVENTORY %d\n", control.button[CONTROL_INVENTORY]);
	fprintf(fp, "PREVIOUS %d\n", control.button[CONTROL_PREVIOUS]);
	fprintf(fp, "NEXT %d\n", control.button[CONTROL_NEXT]);
	fprintf(fp, "ACTIVATE %d\n", control.button[CONTROL_ACTIVATE]);
	fprintf(fp, "INTERACT %d\n", control.button[CONTROL_INTERACT]);
	fprintf(fp, "PAUSE %d\n", control.button[CONTROL_PAUSE]);
	fprintf(fp, "DEAD_ZONE %d\n", control.deadZone);
}

void readControlsFromFile(char *buffer)
{
	char *line, *token, *savePtr;

	savePtr = NULL;

	line = strtok_r(buffer, "\n", &savePtr);

	while (line != NULL)
	{
		token = strtok(line, " ");

		if (strcmpignorecase(token, "LEFT") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_LEFT] = atoi(token);
		}

		else if (strcmpignorecase(token, "RIGHT") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_RIGHT] = atoi(token);
		}

		else if (strcmpignorecase(token, "UP") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_UP] = atoi(token);
		}

		else if (strcmpignorecase(token, "DOWN") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_DOWN] = atoi(token);
		}

		else if (strcmpignorecase(token, "JUMP") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_JUMP] = atoi(token);
		}

		else if (strcmpignorecase(token, "ATTACK") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_ATTACK] = atoi(token);
		}

		else if (strcmpignorecase(token, "BLOCK") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_BLOCK] = atoi(token);
		}

		else if (strcmpignorecase(token, "PREVIOUS") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_PREVIOUS] = atoi(token);
		}

		else if (strcmpignorecase(token, "NEXT") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_NEXT] = atoi(token);
		}

		else if (strcmpignorecase(token, "ACTIVATE") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_ACTIVATE] = atoi(token);
		}

		else if (strcmpignorecase(token, "INTERACT") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_INTERACT] = atoi(token);
		}

		else if (strcmpignorecase(token, "PAUSE") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_PAUSE] = atoi(token);
		}

		else if (strcmpignorecase(token, "INVENTORY") == 0)
		{
			token = strtok(NULL, "\0");

			control.button[CONTROL_INVENTORY] = atoi(token);
		}

		else if (strcmpignorecase(token, "DEAD_ZONE") == 0)
		{
			token = strtok(NULL, "\0");

			control.deadZone = atoi(token);
		}

		else if (strcmpignorecase(token, "GAME_SETTINGS") == 0)
		{
			readGameSettingsFromFile(savePtr);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}
}

char *getKeyValue(int key)
{
	static char text[MAX_VALUE_LENGTH];

	if (key == -1)
	{
		STRNCPY(text, "?", MAX_VALUE_LENGTH);
	}

	else if (key < 0)
	{
		STRNCPY(text, getJoystickButton(key), MAX_VALUE_LENGTH);
	}

	else
	{
		STRNCPY(text, SDL_GetKeyName(key), MAX_VALUE_LENGTH);
	}

	return text;
}

static char *getJoystickButton(int val)
{
	static char text[MAX_VALUE_LENGTH];

	val = abs(val) - 1000;

	snprintf(text, MAX_VALUE_LENGTH, _("Joy Button #%d"), val);

	return text;
}
