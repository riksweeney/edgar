#include "headers.h"

#include "entity.h"

static Trigger trigger[MAX_TRIGGERS];

void addTrigger(char *triggerName, int count, int targetType, char *targetName)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == NOT_IN_USE)
		{
			trigger[i].inUse = IN_USE;

			trigger[i].count = count;
			trigger[i].targetType = targetType;

			strcpy(trigger[i].triggerName, triggerName);
			strcpy(trigger[i].targetName, targetName);

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
		if (trigger[i].inUse == IN_USE && strcmpignorecase(trigger[i].targetName, name) == 0)
		{
			trigger[i].count--;

			if (trigger[i].count <= 0)
			{
				switch (trigger[i].targetType)
				{
					case UPDATE_OBJECTIVE:
						/*updateObjective(trigger[i].targetName);*/
					break;

					case ACTIVATE_ENTITY:
						activateEntitiesWithName(trigger[i].targetName, ACTIVE);
					break;

					default:

					break;
				}

				trigger[i].inUse = NOT_IN_USE;

				return;
			}
		}
	}
}
