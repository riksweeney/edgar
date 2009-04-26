#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"

extern Entity *self, player;

static void die(void);
static void pain(void);
static void wait(void);
static void attack(void);

Entity *addJumpingSlime(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Jumping Slime\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void die()
{
	entityDie();
}

static void pain()
{
	playSound("sound/enemy/bat/squeak.wav", ENEMY_CHANNEL_1, ENEMY_CHANNEL_2, self->x, self->y);
}

static void wait()
{
	if (prand() % 4 == 0)
	{
		if (collision(self->x - 160, self->y, 320 + self->w, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			printf("Spotted player\n");
			
			self->action = &attack;
		}
	}
	
	checkToMap(self);
}

static void attack()
{
	int onGround = (self->flags & ON_GROUND);
	
	if (self->flags & ON_GROUND)
	{
		if (self->thinkTime == 0)
		{
			self->dirY = -(12 + prand() % 4);
			
			self->dirX = player.x < self->x ? -(prand() % 1 + 2) : (prand() % 1 + 2);
		}
		
		else
		{
			self->thinkTime--;
		}
	}
	
	checkToMap(self);
	
	if (onGround == 0 && self->flags & ON_GROUND)
	{
		self->thinkTime = 30 + prand () % 150;
		
		self->dirX = 0;
	}
}
