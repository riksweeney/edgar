Entity *addDrill(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Drill\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ITEM;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->animSpeed = self->active == TRUE ? 1 : 0;
}

static void activate(int val)
{
	if (self->health == 0)
	{
		self->health = 1;

		self->thinkTime = 120;

		self->action = &attack;

		self->flags |= ATTACKING;
	}
}

static void touch(Entity *other)
{

}

static void attack()
{
	if (self->y < self->endY)
	{
		self->y += self->speed;
	}

	else
	{
		self->y = self->endY;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &reset;
		}
	}
}

static void reset()
{
	if (self->y > self->startX)
	{
		self->y -= self->speed;
	}

	else
	{
		self->y = self->startX;

		init();
	}
}

static void init()
{
	self->health = 0;
	self->thinkTime = 0;

	self->action = &wait;

	self->flags &= ~ATTACKING;
}
