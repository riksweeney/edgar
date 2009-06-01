#include "../headers.h"

#include "widget.h"
#include "../init.h"
#include "../graphics/graphics.h"
#include "options_menu.h"
#include "../system/pak.h"
#include "../input.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static void redefineKey(void);

void drawControlMenu()
{
	int i, x, y;

	x = (SCREEN_WIDTH - menu.background->w) / 2;
	y = (SCREEN_HEIGHT - menu.background->h) / 2;

	drawImage(menu.background, x, y, FALSE);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], menu.index == i);
	}
}

static void doMenu()
{
	Widget *w;

	if (input.down == TRUE || menuInput.down == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 0;
		}

		menuInput.down = FALSE;
		input.down = FALSE;
	}

	else if (input.up == TRUE || menuInput.up == TRUE)
	{
		menu.index--;

		if (menu.index < 0)
		{
			menu.index = menu.widgetCount - 1;
		}

		menuInput.up = FALSE;
		input.up = FALSE;
	}

	else if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->action != NULL)
		{
			w->action();
		}

		menuInput.attack = FALSE;
		input.attack = FALSE;
	}
}

static void loadMenuLayout()
{
	char filename[MAX_LINE_LENGTH], *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i;
	SDL_Surface *temp;

	i = 0;

	snprintf(filename, sizeof(filename), _("data/menu/control_menu.dat"));

	buffer = loadFileFromPak(filename);

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	while (line != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr1);

			continue;
		}

		token = strtok_r(line, " ", &savePtr2);

		if (strcmpignorecase(token, "WIDTH") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.w = atoi(token);
		}

		else if (strcmpignorecase(token, "HEIGHT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.h = atoi(token);
		}

		else if (strcmpignorecase(token, "WIDGET_COUNT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.widgetCount = atoi(token);

			menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				printf("Ran out of memory when creating Control Menu\n");

				exit(1);
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "UP") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_UP], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "DOWN") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_DOWN], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "LEFT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_LEFT], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "RIGHT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_RIGHT], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "ATTACK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_ATTACK], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "BLOCK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_BLOCK], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "JUMP") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_JUMP], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "INTERACT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_INTERACT], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "USE") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_ACTIVATE], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "PREV_ITEM") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_PREVIOUS], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "NEXT_ITEM") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_NEXT], 0, 0, &redefineKey, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_BACK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, &showOptionsMenu, x, y);
				}

				else
				{
					printf("Unknown widget %s\n", menuID);

					exit(1);
				}

				i++;
			}

			else
			{
				printf("Widget Count must be defined!\n");

				exit(1);
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w == 0 || menu.h == 0)
	{
		printf("Menu dimensions must be greater than 0\n");

		exit(1);
	}

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, menu.w, menu.h, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	menu.background = addBorder(SDL_DisplayFormat(temp), 255, 255, 255, 0, 0, 0);

	SDL_FreeSurface(temp);

	free(buffer);
}

Menu *initControlMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	return &menu;
}

void freeControlMenu()
{
	int i;

	if (menu.widgets != NULL)
	{
		for (i=0;i<menu.widgetCount;i++)
		{
			freeWidget(menu.widgets[i]);
		}

		free(menu.widgets);
	}

	if (menu.background != NULL)
	{
		SDL_FreeSurface(menu.background);

		menu.background = NULL;
	}
}

static void redefineKey()
{
	int key = getSingleInput();
	Widget *w = menu.widgets[menu.index];

	if (key != -1)
	{
		(*w->value) = key;
	}
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}
