#include "../headers.h"

#include "../graphics/animation.h"
#include "../inventory.h"
#include "../entity.h"
#include "../hud.h"
#include "../system/properties.h"
#include "../audio/audio.h"

extern Entity *self;

static void activate(int);
static void call(int);
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
	e->activate = strcmpignorecase(name, "common/call_switch") == 0 ? &call : &activate;

	e->type = SWITCH;

	setEntityAnimation(e, STAND);

	return e;
}

static void call(int val)
{
	Entity *e, *temp;
	
	if (self->thinkTime == 0)
	{
		if (strlen(self->requires) != 0)
		{
			printf("Requires %s\n", self->requires);
	
			if (removeInventoryItem(self->requires) == 1)
			{
				self->requires[0] = '\0';
			}
	
			else
			{
				setInfoBoxMessage(120,  _("%s is needed to activate this switch"), self->requires);
	
				return;
			}
		}
	
		playSound("sound/common/switch.wav", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, self->x, self->y);
	
		self->active = TRUE;
	
		setEntityAnimation(self, WALK);
		
		self->thinkTime = 120;
	
		e = getEntityByObjectiveName(self->objectiveName);
		
		if (e != NULL)
		{
			temp = self;
			
			self = e;
			
			self->activate(temp->health - self->health);
			
			self = temp;
		}
		
		else
		{
			printf("Could not find an Entity called %s\n", self->objectiveName);
			
			exit(1);
		}
	}
}

static void activate(int val)
{
	if (strlen(self->requires) != 0)
	{
		printf("Requires %s\n", self->requires);

		if (removeInventoryItem(self->requires) == 1)
		{
			self->requires[0] = '\0';
		}

		else
		{
			setInfoBoxMessage(120,  _("%s is needed to activate this switch"), self->requires);

			return;
		}
	}

	playSound("sound/common/switch.wav", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, self->x, self->y);

	self->active = self->active == TRUE ? FALSE : TRUE;

	setEntityAnimation(self, self->active == TRUE ? WALK : STAND);

	printf("Activating entities with name %s\n", self->objectiveName);

	activateEntitiesWithName(self->objectiveName, self->active);
}

static void wait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime == 0)
		{
			self->active = FALSE;
			
			playSound("sound/common/switch.wav", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, self->x, self->y);
			
			setEntityAnimation(self, STAND);
		}
	}
}

static void initialise()
{
	setEntityAnimation(self, self->active == TRUE ? WALK : STAND);

	self->action = &wait;
}
