#include "../headers.h"

#include "../animation.h"
#include "../properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../game.h"
#include "../audio.h"
#include "../music.h"
#include "../map.h"
#include "../random.h"
#include "../custom_actions.h"
#include "../projectile.h"

extern Entity *self, player;

static void initialise(void);

Entity *addGolemBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Golem Boss\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();

	if (self->active == TRUE)
	{
		adjustMusicVolume(-1);

		centerMapOnEntity(NULL);

		if (minX < self->endX)
		{
			minX++;
		}

		else if (minX > self->endX)
		{
			minX--;
		}

		if (minY < self->endY)
		{
			minY++;
		}

		else if (minY > self->endY)
		{
			minY--;
		}

		setMapStartX(minX);
		setMapStartY(minY);

		if (minX == self->endX && minY == self->endY)
		{
			self->dirX = self->speed;

			/*setEntityAnimation(self, ATTACK_2);*/

			self->action = &doIntro;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			printf("Starting\n");
		}
	}
}

static void doIntro()
{

}
