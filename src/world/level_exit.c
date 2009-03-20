#include "../headers.h"

#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/properties.h"
#include "../player.h"
#include "../game.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void wait(void);
static void touch(Entity *);
static void activate(int);
static void init(void);

Entity *addLevelExit(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Level Exit\n");

		exit(1);
	}

	loadProperties("common/level_exit", e);

	e->x = x;
	e->y = y;

	e->endX = x;

	STRNCPY(e->name, name, sizeof(e->name));

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->activate = &activate;
	e->thinkTime = 60;

	e->type = LEVEL_EXIT;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->dirX = (self->face == RIGHT ? 10 : -10);

	self->action = &wait;

	self->action();
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		self->x -= self->dirX * 2;
	}

	else if (self->thinkTime % 20 == 0)
	{
		self->x += self->dirX;
	}
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0,  _("Press Action to go to the %s"), self->requires);
	}
}

static void activate(int val)
{
	player.flags |= HELPLESS;
	
	setEntityAnimation(&player, STAND);

	player.dirX = 0;

	setNextLevel(self->name, self->objectiveName);

	setTransition(TRANSITION_OUT, &goToNextMap);
}
