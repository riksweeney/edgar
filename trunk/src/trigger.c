#include "headers.h"

#include "entity.h"
#include "objective.h"

static Trigger trigger[MAX_TRIGGERS];

void addTrigger(char *triggerName, int count, int targetType, char *targetName)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == FALSE)
		{
			trigger[i].inUse = TRUE;

			trigger[i].count = count;
			trigger[i].targetType = targetType;

			strcpy(trigger[i].triggerName, triggerName);
			strcpy(trigger[i].targetName, targetName);

			printf("Added trigger %s with count %d\n", trigger[i].triggerName, trigger[i].count);

			return;
		}
	}

	printf("No free slots to add Trigger %s\n", triggerName);

	exit(1);
}

void fireTrigger(char *name)
{
	int i;

	if (strlen(name) == 0)
	{
		return;
	}

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE && strcmpignorecase(trigger[i].triggerName, name) == 0)
		{
			trigger[i].count--;

			if (trigger[i].count <= 0)
			{
				printf("Firing trigger %s\n", trigger[i].triggerName);

				switch (trigger[i].targetType)
				{
					case UPDATE_OBJECTIVE:
						updateObjective(trigger[i].targetName);
					break;

					case ACTIVATE_ENTITY:
						activateEntitiesWithName(trigger[i].targetName, TRUE);
					break;

					case UPDATE_BOTH:
						activateEntitiesWithName(trigger[i].targetName, TRUE);

						updateObjective(trigger[i].targetName);
					break;

					default:

					break;
				}

				trigger[i].inUse = FALSE;

				return;
			}
		}
	}
}
