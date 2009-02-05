#include "headers.h"

#include "animation.h"
#include "inventory.h"
#include "entity.h"
#include "hud.h"
#include "properties.h"
#include "audio.h"

extern Entity *self;

static void activate(int);
static void wait(void);
static void initialise(void);

Entity *addSwitch(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Switch\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->action = &initialise;
	e->activate = &activate;

	e->type = SWITCH;

	setEntityAnimation(e, STAND);

	return e;
}

static void activate(int val)
{
	if (strlen(self->requires) != 0)
	{
		if (removeInventoryItem(self->requires) == 1)
		{
			strcpy(self->requires, "");
		}
		
		else
		{
			addHudMessage("%s is needed to activate this switch", self->requires);
			
			return;
		}
	}
	
	playSound("sound/common/switch.wav", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, self->x, self->y);
	
	self->active = self->active == ACTIVE ? INACTIVE : ACTIVE;
	
	setEntityAnimation(self, self->active == ACTIVE ? WALK : STAND);
	
	printf("Activating entities with name %s\n", self->objectiveName);
	
	activateEntitiesWithName(self->objectiveName, self->active);
}

static void wait()
{

}

static void initialise()
{
	setEntityAnimation(self, self->active == ACTIVE ? WALK : STAND);
	
	self->action = &wait;
}
