#include "headers.h"

#include "hud.h"
#include "trigger.h"

static Objective objective[MAX_OBJECTIVES];

void clearObjectives()
{
	/* Clear the list */

	memset(objective, 0, sizeof(Objective) * MAX_OBJECTIVES);
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

			strcpy(objective[i].name, objectiveName);
			strcpy(objective[i].completionTrigger, completionTrigger);

			return;
		}
	}

	printf("No free slots to add Objective %s\n", objectiveName);

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
				printf("Completing objective %s\n", objective[i].name);

				addHudMessage(GOOD_MESSAGE, "Objective Completed: %s", objective[i].name);

				fireTrigger(objective[i].completionTrigger);

				objective[i].inUse = FALSE;
			}
		}
	}
}
