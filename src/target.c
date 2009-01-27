#include "headers.h"

extern Target target[MAX_TARGETS];
extern Entity *self;

static Entity targetEntity;

void clearTargets()
{
	/* Clear the list */

	memset(target, 0, sizeof(Target) * MAX_TARGETS);
}

Target *addTarget(int x, int y, char *name)
{
	int i;

	loadProperties("lift/lift_target", &targetEntity);

	targetEntity.draw = &drawLoopingAnimationToMap;

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (strcmpignorecase(name, target[i].name) == 0)
		{
			printf("Duplicate target name %s\n", name);

			exit(1);
		}
	}

	/* Loop through all the targets and find a free slot */

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (target[i].active == INACTIVE)
		{
			memset(&target[i], 0, sizeof(Target));

			target[i].active = ACTIVE;

			target[i].x = x;
			target[i].y = y;

			strcpy(target[i].name, name);

			return &target[i];
		}
	}

	printf("No free slots to add a target\n");

	exit(1);
}

void drawTargets()
{
	int i;

	self = &targetEntity;

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (target[i].active == ACTIVE)
		{
			self->x = target[i].x;
			self->y = target[i].y;

			self->draw();
		}
	}
}

Target *getTargetByName(char *name)
{
	int i;

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (target[i].active == ACTIVE && strcmpignorecase(target[i].name, name) == 0)
		{
			return &target[i];
		}
	}

	return NULL;
}
