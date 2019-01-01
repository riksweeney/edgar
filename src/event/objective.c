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

#include "../entity.h"
#include "../hud.h"
#include "../system/error.h"
#include "global_trigger.h"
#include "objective.h"
#include "trigger.h"

static Objective objective[MAX_OBJECTIVES];

static void addObjective(char *, char *);

void freeObjectives()
{
	/* Clear the list */

	memset(objective, 0, sizeof(Objective) * MAX_OBJECTIVES);
}

void addObjectiveFromScript(char *line)
{
	char objectiveName[MAX_MESSAGE_LENGTH], completionTrigger[MAX_VALUE_LENGTH];

	sscanf(line, "\"%[^\"]\" \"%[^\"]\"", objectiveName, completionTrigger);

	addObjective(objectiveName, completionTrigger);
}

void addObjectiveFromResource(char *key[], char *value[])
{
	int i, objectiveName, completionTrigger;

	objectiveName = completionTrigger = -1;

	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		if (strcmpignorecase("OBJECTIVE_NAME", key[i]) == 0)
		{
			objectiveName = i;
		}

		else if (strcmpignorecase("COMPLETION_TRIGGER", key[i]) == 0)
		{
			completionTrigger = i;
		}
	}

	if (objectiveName == -1)
	{
		showErrorAndExit("Objective name is missing");
	}

	addObjective(value[objectiveName], completionTrigger == -1 ? "" : value[completionTrigger]);
}

void addObjective(char *objectiveName, char *completionTrigger)
{
	int i;

	if (strcmpignorecase(objectiveName, "Create a Disintegration Shield") == 0)
	{
		return;
	}

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		/* Don't add duplicates */

		if (objective[i].inUse == TRUE && strcmpignorecase(objective[i].name, objectiveName) == 0)
		{
			return;
		}

		if (objective[i].inUse == FALSE)
		{
			objective[i].inUse = TRUE;
			objective[i].active = TRUE;
			objective[i].completed = FALSE;

			STRNCPY(objective[i].name, objectiveName, sizeof(objective[i].name));
			STRNCPY(objective[i].completionTrigger, completionTrigger, sizeof(objective[i].completionTrigger));

			setInfoBoxMessage(60, 255, 255, 255, _("New Objective: %s"), _(objective[i].name));

			return;
		}
	}

	showErrorAndExit("No free slots to add Objective \"%s\"", objectiveName);
}

void updateObjective(char *objectiveName)
{
	int i;

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		if (objective[i].inUse == TRUE && objective[i].active == TRUE && objective[i].completed == FALSE)
		{
			if (strcmpignorecase(objective[i].name, objectiveName) == 0)
			{
				freeMessageQueue();

				setInfoBoxMessage(180, 0, 255, 0, _("Objective Completed: %s"), _(objective[i].name));

				fireTrigger(objective[i].completionTrigger);

				fireGlobalTrigger(objective[i].completionTrigger);

				objective[i].inUse = FALSE;
			}
		}
	}
}

void modifyObjective(char *objectiveName, char *completionTrigger)
{
	int i;

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		if (objective[i].inUse == TRUE && objective[i].active == TRUE && objective[i].completed == FALSE)
		{
			if (strcmpignorecase(objective[i].name, objectiveName) == 0)
			{
				STRNCPY(objective[i].completionTrigger, completionTrigger, sizeof(objective[i].completionTrigger));

				break;
			}
		}
	}
}

void removeObjective(char *objectiveName)
{
	int i;

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		if (strcmpignorecase(objective[i].name, objectiveName) == 0)
		{
			objective[i].inUse = FALSE;

			break;
		}
	}
}

void getObjectiveFromScript(char *line)
{
	char command[15], objectiveName[MAX_VALUE_LENGTH], entityName[MAX_VALUE_LENGTH];
	int i, success, failure, found;
	Entity *e;

	sscanf(line, "%s \"%[^\"]\" %s %d %d", command, objectiveName, entityName, &success, &failure);

	e = getEntityByObjectiveName(entityName);

	if (e == NULL)
	{
		showErrorAndExit("Could not find Entity %s to check Objective %s against", entityName, objectiveName);
	}

	found = FALSE;

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		if (objective[i].inUse == TRUE && strcmpignorecase(objective[i].name, objectiveName) == 0)
		{
			found = TRUE;

			break;
		}
	}

	e->health = found == TRUE ? success : failure;
}

void writeObjectivesToFile(FILE *fp)
{
	int i;

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		if (objective[i].inUse == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE OBJECTIVE\n");
			fprintf(fp, "OBJECTIVE_NAME %s\n", objective[i].name);
			fprintf(fp, "COMPLETION_TRIGGER %s\n", objective[i].completionTrigger);
			fprintf(fp, "}\n\n");
		}
	}
}
