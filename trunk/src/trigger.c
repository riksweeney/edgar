#include "trigger.h"

extern void updateObjective(char *);

void addTrigger(char *name, int count, int targetType, int targetName)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == NOT_IN_USE)
		{
			trigger[i].inUse = IN_USE;

			trigger[i].count = count;
			trigger[i].targetType = targetType;

			strcpy(trigger[i].name, name);
			strcpy(trigger[i].targetName, targetName);

			return;
		}
	}

	printf("No free slots to add a Trigger\n");

	exit(1);
}

void fireTrigger(char *name)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == IN_USE && strcmpignorecase(trigger[i].name, name) == 0)
		{
			trigger[i].count--;

			if (trigger[i].count <= 0)
			{
				switch (trigger[i].targetType)
				{
					case UPDATE_OBJECTIVE:
						updateObjective(trigger[i].targetName);
					break;

					default:
						printf("Trigger Target Type %d does not exist\n");

						exit(1);
					break;
				}

				trigger[i].inUse = NOT_IN_USE;
			}

			return;
		}
	}
}
