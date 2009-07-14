#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../system/random.h"
#include "../audio/audio.h"

extern Entity *self;

static void touch(Entity *);
static void shatter(void);
static void wait(void);
static void explode(void);

Entity *addSnailShell(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Snail Shell\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;
	
	e->action = &wait;
	e->die = &shatter;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime--;
	
	if (self->thinkTime < 120)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= FLASH;
		}
	}
	
	else if (self->thinkTime < 180)
	{
		if (self->thinkTime % 6 == 0)
		{
			self->flags ^= FLASH;
		}
	}
	
	if (self->thinkTime <= 0)
	{
		self->action = &explode;
	}
	
	checkToMap(self);
}

static void touch(Entity *other)
{
	pushEntity(other);

	if ((other->flags & ATTACKING) && !(self->flags & INVULNERABLE))
	{
		entityTakeDamageNoFlinch(other, other->damage);
	}
}

static void explode()
{
	Entity *e;
	char name[MAX_VALUE_LENGTH];
	
	playSoundToMap("sound/common/explosion.ogg", -1, self->x, self->y, 0);
	
	snprintf(name, sizeof(name), "%s_piece", self->name);

	e = addProjectile(name, self, self->x, self->y, -12, 0);
	
	e->parent = e;
	
	e->reactToBlock = &bounceOffShield;
	
	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;
	
	e->flags |= FLY;
	
	setEntityAnimation(e, 0);
	
	e->parent = e;
	
	e = addProjectile(name, self, self->x, self->y, 12, 0);
	
	e->parent = e;
	
	e->reactToBlock = &bounceOffShield;
	
	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;
	
	e->flags |= FLY;
	
	setEntityAnimation(e, 1);
	
	e->parent = e;
	
	e = addProjectile(name, self, self->x, self->y, -12, -6);
	
	e->parent = e;
	
	e->reactToBlock = &bounceOffShield;
	
	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;
	
	e->flags |= FLY;
	
	setEntityAnimation(e, 2);
	
	e->parent = e;
	
	e = addProjectile(name, self, self->x, self->y, 12, -6);
	
	e->parent = e;
	
	e->reactToBlock = &bounceOffShield;
	
	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;
	
	e->flags |= FLY;
	
	setEntityAnimation(e, 3);
	
	e->parent = e;
	
	self->inUse = FALSE;
}

static void shatter()
{
	int i;
	Entity *e;

	for (i=0;i<4;i++)
	{
		if (strcmpignorecase(self->name, "enemy/purple_snail_shell") == 0)
		{
			e = addTemporaryItem("enemy/purple_snail_shell_piece", self->x, self->y, RIGHT, 0, 0);
		}
		
		else
		{
			e = addTemporaryItem("enemy/snail_shell_piece", self->x, self->y, RIGHT, 0, 0);
		}

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimation(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}

