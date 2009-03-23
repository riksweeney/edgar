#include "../headers.h"

#include "widget.h"
#include "../init.h"
#include "../graphics/graphics.h"

extern Input input, menuInput;
extern Game game;

static Menu mainMenu;

static void loadMainMenuLayout(void);

void drawMainMenu()
{
	int i, x, y;

	x = (SCREEN_WIDTH - mainMenu.background->w) / 2;
	y = (SCREEN_HEIGHT - mainMenu.background->h) / 2;
	
	drawImage(mainMenu.background, x, y, FALSE);

	for (i=0;i<MAX_MAIN_MENU_OPTIONS;i++)
	{
		drawWidget(mainMenu.widgets[i], mainMenu.index == i);
	}
}

void doMainMenu()
{
	Widget *w;
	
	if (mainMenu.widgets == NULL)
	{
		loadMainMenuLayout();
	}

	if (input.down == TRUE || menuInput.down == TRUE)
	{
		mainMenu.index++;

		if (mainMenu.index == MAX_MAIN_MENU_OPTIONS)
		{
			mainMenu.index = 0;
		}

		menuInput.down = FALSE;
		input.down = FALSE;
	}

	else if (input.up == TRUE || menuInput.up == TRUE)
	{
		mainMenu.index--;

		if (mainMenu.index < 0)
		{
			mainMenu.index = MAX_MAIN_MENU_OPTIONS - 1;
		}

		menuInput.up = FALSE;
		input.up = FALSE;
	}

	else if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		w = mainMenu.widgets[mainMenu.index];

		if (w->action != NULL)
		{
			w->action();
		}
		
		menuInput.attack = FALSE;
		input.attack = FALSE;
	}
}

static void loadMainMenuLayout()
{
	char line[MAX_LINE_LENGTH], menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token;
	int x, y;
	FILE *fp;
	SDL_Surface *temp;

	mainMenu.widgets = (Widget **)malloc(sizeof(Widget *) * MAX_MAIN_MENU_OPTIONS);

	if (mainMenu.widgets == NULL)
	{
		printf("Ran out of memory when creating Main Menu\n");

		exit(1);
	}
	
	snprintf(line, sizeof(line), _("%sdata/menu/main_menu.dat"), INSTALL_PATH);

	fp = fopen(line, "rb");

	if (fp == NULL)
	{
		perror("Could not open Main Menu config file");

		exit(1);
	}

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		token = strtok(line, " ");
		
		if (strcmpignorecase(token, "WIDTH") == 0)
		{
			token = strtok(NULL, " ");
			
			mainMenu.w = atoi(token);
		}
		
		else if (strcmpignorecase(token, "HEIGHT") == 0)
		{
			token = strtok(NULL, " ");
			
			mainMenu.h = atoi(token);
		}
		
		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			token = strtok(NULL, "\0");
			
			sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);
	
			if (strcmpignorecase(menuID, "MENU_NEW_GAME") == 0)
			{
				mainMenu.widgets[MENU_NEW_GAME] = createWidget(menuName, NULL, 0, 0, NULL, x, y);
			}
	
			else if (strcmpignorecase(menuID, "MENU_LOAD") == 0)
			{
				mainMenu.widgets[MENU_LOAD] = createWidget(menuName, NULL, 0, 0, NULL, x, y);
			}
	
			else if (strcmpignorecase(menuID, "MENU_OPTIONS") == 0)
			{
				mainMenu.widgets[MENU_OPTIONS] = createWidget(menuName, NULL, 0, 0, NULL, x, y);
			}
	
			else if (strcmpignorecase(menuID, "MENU_ABOUT") == 0)
			{
				mainMenu.widgets[MENU_ABOUT] = createWidget(menuName, NULL, 0, 0, NULL, x, y);
			}
	
			else if (strcmpignorecase(menuID, "MENU_QUIT") == 0)
			{
				mainMenu.widgets[MENU_QUIT] = createWidget(menuName, NULL, 0, 0, &quitGame, x, y);
			}
		}
	}
	
	if (mainMenu.w == 0 || mainMenu.h == 0)
	{
		printf("Menu dimensions must be greater than 0\n");
		
		exit(1);
	}
	
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, mainMenu.w, mainMenu.h, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	mainMenu.background = addBorder(SDL_DisplayFormat(temp), 255, 255, 255, 0, 0, 0);
	
	SDL_FreeSurface(temp);

	fclose(fp);
}

void freeMainMenu()
{
	int i;
	
	if (mainMenu.widgets != NULL)
	{
		for (i=0;i<MAX_MAIN_MENU_OPTIONS;i++)
		{
			freeWidget(mainMenu.widgets[i]);
		}
	
		free(mainMenu.widgets);
	}
	
	if (mainMenu.background != NULL)
	{
		SDL_FreeSurface(mainMenu.background);
		
		mainMenu.background = NULL;
	}
}
