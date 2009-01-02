#include "trigger.h"

void addTrigger(int x, int y, int w, int y, char *activates)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Failed to add Trigger\n");
		
		exit(1);
	}
	
	e->x = x;
	e->y = y;
	e->w = w;
	e->h = h;
	
	strcpy(e->
}
