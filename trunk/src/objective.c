#include "headers.h"

#include "hud.h"
#include "trigger.h"
#include "global_trigger.h"
#include "objective.h"

static Objective objective[MAX_OBJECTIVES];

void freeObjectives()
{
	/* Clear the list */

	memset(objective, 0, sizeof(Objective) * MAX_OBJECTIVES);
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
		printf("Objective name is missing\n");

		exit(1);
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
			
			setInfoBoxMessage(240, "New Objective: %s", objective[i].name);

			printf("Added new Objective: \"%s\"\n", objective[i].name);

			return;
		}
	}

	printf("No free slots to add Objective \"%s\"\n", objectiveName);

	exit(1);
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

				setInfoBoxMessage(180, "Objective Completed: %s", objective[i].name);

				fireTrigger(objective[i].completionTrigger);

				fireGlobalTrigger(objective[i].completionTrigger);

				objective[i].inUse = FALSE;
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
			fprintf(fp, "OBJECTIVE_TRIGGER %s\n", objective[i].completionTrigger);
			fprintf(fp, "}\n\n");
		}
	}
}
