/* This tutorial demonstrates tile based collision
** detection. Pressing the arrow keys
** will sprite around the screen. Pressing escape
** or closing the window will exit the program
*/

#include "main_editor.h"

extern void init(char *);
extern void cleanup(void);
extern void getInput(void);
extern void loadMap(char *);
extern void draw(void);
extern void doCursor(void);
extern void delay(unsigned int);
extern void doStatusPanel(void);
extern void loadMapBackground(char *);
extern void loadResources(void);
extern void initCursor(void);
extern void setMaxMapX(int);
extern void setMaxMapY(int);
extern void centerMapOnEntity(Entity *);

int main(int argc, char *argv[])
{
	unsigned int frameLimit = SDL_GetTicks() + 16;
	int go;
	
	/* Start up SDL */
	
	init("Map Editor");
	
	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = 1;

	/* Load the resources */
	
	loadResources();
	
	/* Load the background image */
	
	setMaxMapX(MAX_MAP_X * TILE_SIZE);
	setMaxMapY(MAX_MAP_Y * TILE_SIZE);
	
	/* Remove auto targeting */
	
	centerMapOnEntity(NULL);
	
	/* Initialise the cursor */
	
	initCursor();
	
	/* Loop indefinitely for messages */
	
	while (go == 1)
	{
		/* Get the input */
	
		getInput();
		
		/* Do the cursor */
		
		doCursor();
		
		/* Do the status panel */
		
		doStatusPanel();
		
		/* Draw the map */
		
		draw();
		
		/* Sleep briefly to stop sucking up all the CPU time */
		
		delay(frameLimit);
		
		frameLimit = SDL_GetTicks() + 16;
	}
	
	/* Exit the program */
	
	exit(0);
}
