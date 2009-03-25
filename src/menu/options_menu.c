#include "../headers.h"

#include "widget.h"
#include "../init.h"
#include "../graphics/graphics.h"
#include "main_menu.h"

extern Input input, menuInput;
extern Game game;

static Menu menu;

static void loadMenuLayout(void);
static void toggleHints(void);
static void showControlMenu(void);
static void showSoundMenu(void);
static void showMainMenu(void);
static void doMenu(void);

void drawOptionsMenu()
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
	char line[MAX_LINE_LENGTH], menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token;
	int x, y, i;
	FILE *fp;
	SDL_Surface *temp;

	i = 0;

	snprintf(line, sizeof(line), _("%sdata/menu/options_menu.dat"), INSTALL_PATH);

	fp = fopen(line, "rb");

	if (fp == NULL)
	{
		perror("Could not open Options Menu config file");

		exit(1);
	}

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		token = strtok(line, " ");

		if (strcmpignorecase(token, "WIDTH") == 0)
		{
			token = strtok(NULL, " ");

			menu.w = atoi(token);
		}

		else if (strcmpignorecase(token, "HEIGHT") == 0)
		{
			token = strtok(NULL, " ");

			menu.h = atoi(token);
		}

		else if (strcmpignorecase(token, "WIDGET_COUNT") == 0)
		{
			token = strtok(NULL, " ");

			menu.widgetCount = atoi(token);

			menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				printf("Ran out of memory when creating Options Menu\n");

				exit(1);
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok(NULL, "\0");

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "MENU_CONTROLS") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, &showControlMenu, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_SOUND") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, &showSoundMenu, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_HINTS") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, &toggleHints, x, y);
				}

				else if (strcmpignorecase(menuID, "MENU_BACK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, 0, 0, &showMainMenu, x, y);
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
	}

	if (menu.w == 0 || menu.h == 0)
	{
		printf("Menu dimensions must be greater than 0\n");

		exit(1);
	}

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, menu.w, menu.h, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	menu.background = addBorder(SDL_DisplayFormat(temp), 255, 255, 255, 0, 0, 0);

	SDL_FreeSurface(temp);

	fclose(fp);
}

Menu *initOptionsMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	return &menu;
}

void freeOptionsMenu()
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

static void toggleHints()
{
	game.showHints = game.showHints == TRUE ? FALSE : TRUE;
}

static void showControlMenu()
{
	/*
	game.menu = initControlMenu();

	game.drawMenu = &drawControlMenu;
	*/
}

static void showSoundMenu()
{
	/*
	game.menu = initSoundMenu();

	game.drawMenu = &drawShowMenu;
	*/
}

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}
