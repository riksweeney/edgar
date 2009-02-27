Entity *addShrub(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Shrub\n");

		exit(1);
	}

	loadProperties("misc/shrub", e);

	e->x = x;
	e->y = y;

	e->action = &doNothing;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;
	e->takeDamage = &takeDamage;

	e->type = KEY_ITEM;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if ((other->flags & ATTACKING) && strcmpignorecase(other->name, self->requires) == 0)
	{
		self->health--;

		if (self->health <= 0)
		{
			self->action = &die;
		}
	}
}

static void die()
{
	self->inUse = FALSE;
}
