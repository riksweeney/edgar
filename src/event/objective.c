/*
Copyright (C) 2009-2010 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../hud.h"
#include "trigger.h"
#include "global_trigger.h"
#include "objective.h"
#include "../system/error.h"

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

	for (i=0;i<MAX_OBJECTIVES;i++)
	{
		if (objective[i].inUse == FALSE)
		{
			objective[i].inUse = TRUE;
			objective[i].active = TRUE;
			objective[i].completed = FALSE;

			STRNCPY(objective[i].name, objectiveName, sizeof(objective[i].name));
			STRNCPY(objective[i].completionTrigger, completionTrigger, sizeof(objective[i].completionTrigger));

			setInfoBoxMessage(60, _("New Objective: %s"), _(objective[i].name));

			printf("Added new Objective: \"%s\" with trigger \"%s\"\n", objective[i].name, objective[i].completionTrigger);

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
				printf("Completing objective \"%s\"\n", objective[i].name);

				freeMessageQueue();

				setInfoBoxMessage(60, _("Objective Completed: %s"), _(objective[i].name));

				printf("Firing triggers with name %s\n", objective[i].completionTrigger);

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
