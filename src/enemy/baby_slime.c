#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "enemies.h"
#include "../custom_actions.h"
#include "../hud.h"

extern Entity *self, player;
extern Game game;

static void stickToPlayer(void);
static void attack(void);
static void grab(Entity *other);
static void fallOff(void);

Entity *addBabySlime(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Baby Slime\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &attack;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &grab;
	e->die = &entityDieNoDrop;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void attack()
{
	int onGround = (self->flags & ON_GROUND);

	self->face = player.x < self->x ? LEFT : RIGHT;

	if ((self->flags & ON_GROUND) && (prand() % 30 == 0))
	{
		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->dirY = -(8 + prand() % 4);
	}

	checkToMap(self);

	if (onGround == 0 && (self->flags & ON_GROUND))
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->die();
	}
}

static void grab(Entity *other)
{
	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}

		other->inUse = FALSE;
	}

	else if (other->type == PLAYER && !(self->flags & GRABBING))
	{
		self->startX += (prand() % (other->w / 2)) * (prand() % 2 == 0 ? 1 : -1);

		self->startY = prand() % (other->h - self->h);

		setCustomAction(other, &slowDown, 3, 1);

		self->action = &stickToPlayer;

		self->touch = NULL;

		self->flags |= (GRABBING|ALWAYS_ON_TOP);

		other->flags |= GRABBED;

		self->thinkTime = 0;

		self->damage = 3 + (prand() % 17);
	}
}

static void stickToPlayer()
{
	setCustomAction(&player, &slowDown, 3, 0);

	if (game.showHints == TRUE)
	{
		setInfoBoxMessage(0,  _("Quickly turn left and right to shake off the slimes!"));
	}

	self->x = player.x + (player.w - self->w) / 2 + self->startX;
	self->y = player.y + self->startY;

	self->thinkTime++;

	if (self->face != player.face)
	{
		self->face = player.face;

		if (self->thinkTime <= 15)
		{
			self->damage--;
		}

		self->thinkTime = 0;
	}

	if (self->damage <= 0)
	{
		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		setCustomAction(&player, &slowDown, 3, -1);

		self->action = &fallOff;

		player.flags &= ~GRABBED;
	}
}

static void fallOff()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->die();
	}
}
