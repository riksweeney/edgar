#include "apple.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern void generalItemAction(void);
extern void healthTouch(Entity *);
extern Entity *getFreeEntity(void);

void addApple(int x, int y)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("No free slots to add an apple\n");
		
		exit(1);
	}
	
	loadProperties("apple", e);
	
	e->x = x;
	e->y = y;
	
	e->dirY = ITEM_JUMP_HEIGHT;
	
	e->thinkTime = 300;
	e->type = HEALTH;
	
	e->face = RIGHT;
	
	e->action = &generalItemAction;
	e->touch = &healthTouch;
	e->draw = &drawLoopingAnimationToMap;
	
	setEntityAnimation(e, STAND);
}
