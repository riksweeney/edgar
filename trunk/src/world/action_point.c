#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../collisions.h"
#include "../audio/audio.h"
#include "../event/script.h"
#include "../hud.h"

extern Game game;
extern Entity *self;

static void wait(void);
static void touch(Entity *);
static void activate(int);

Entity *addActionPoint(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for an Action Point!\n");

		exit(1);
	}
	
	printf("Adding action point\n");

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	e->health = 0;

	e->maxHealth = e->health;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime++;
	
	self->dirY = 0.2 * cos(DEG_TO_RAD(self->thinkTime));
	
	self->y += self->dirY;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0,  _("Press Action to interact"));
	}
}

static void activate(int val)
{
	loadScript(self->requires);

	readNextScriptLine();
}
