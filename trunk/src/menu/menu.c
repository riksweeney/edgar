#include "../headers.h"

#include "main_menu.h"
#include "options_menu.h"

extern Game game;

void doMenu()
{
	game.menu->action();
}

void drawMenu()
{
	game.drawMenu();
}

void freeMenus()
{
	freeMainMenu();
	freeOptionsMenu();
}
