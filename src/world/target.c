/*
Copyright (C) 2009-2019 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static Target target[MAX_TARGETS];
static Entity targetEntity;

void freeTargets()
{
	/* Clear the list */

	memset(target, 0, sizeof(Target) * MAX_TARGETS);
}

Target *addTarget(int x, int y, char *name)
{
	int i;

	targetEntity.inUse = TRUE;

	loadProperties("lift/lift_target", &targetEntity);

	setEntityAnimation(&targetEntity, "STAND");

	targetEntity.draw = &drawLoopingAnimationToMap;

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (strcmpignorecase(name, target[i].name) == 0)
		{
			showErrorAndExit("Duplicate target name %s", name);
		}
	}

	/* Loop through all the targets and find a free slot */

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (target[i].active == FALSE)
		{
			memset(&target[i], 0, sizeof(Target));

			target[i].active = TRUE;

			target[i].x = x;
			target[i].y = y;

			STRNCPY(target[i].name, name, sizeof(target[i].name));

			return &target[i];
		}
	}

	showErrorAndExit("No free slots to add a target");

	return NULL;
}

void drawTargets()
{
	int i;

	self = &targetEntity;

	self->alpha = 255;

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (target[i].active == TRUE)
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
		if (target[i].active == TRUE && strcmpignorecase(target[i].name, name) == 0)
		{
			return &target[i];
		}
	}

	#if DEV == 1
		printf("Could not find target %s\n", name);
	#endif

	return NULL;
}

Target *getTargets()
{
	return target;
}

void writeTargetsToFile(FILE *fp)
{
	int i;

	for (i=0;i<MAX_TARGETS;i++)
	{
		if (target[i].active == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE TARGET\n");
			fprintf(fp, "NAME %s\n", target[i].name);
			fprintf(fp, "START_X %d\n", target[i].x);
			fprintf(fp, "START_Y %d\n", target[i].y);
			fprintf(fp, "}\n\n");
		}
	}
}
