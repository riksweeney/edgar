#include "headers.h"

#include "entity.h"
#include "objective.h"
#include "trigger.h"
#include "global_trigger.h"
#include "script.h"

static Trigger trigger[MAX_TRIGGERS];

static void addGlobalTrigger(char *, int, int, char *);

void freeGlobalTriggers()
{
	/* Clear the list */

	memset(trigger, 0, sizeof(Trigger) * MAX_TRIGGERS);
}

void addGlobalTriggerFromScript(char *line)
{
	char triggerName[MAX_VALUE_LENGTH], targetName[MAX_VALUE_LENGTH], targetType[MAX_VALUE_LENGTH], count[MAX_VALUE_LENGTH];

	sscanf(line, "\"%[^\"]\" %s %s \"%[^\"]\"", triggerName, count, targetType, targetName);

	addGlobalTrigger(triggerName, atoi(count), getTriggerTypeByName(targetType), targetName);
}

void addGlobalTriggerFromResource(char *key[], char *value[])
{
	int i, triggerName, count, targetType, targetName;

	triggerName = count = targetType = targetName = -1;

	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		if (strcmpignorecase("TRIGGER_NAME", key[i]) == 0)
		{
			triggerName = i;
		}

		else if (strcmpignorecase("TRIGGER_COUNT", key[i]) == 0)
		{
			count = i;
		}

		else if (strcmpignorecase("TRIGGER_TYPE", key[i]) == 0)
		{
			targetType = i;
		}

		else if (strcmpignorecase("TRIGGER_TARGET", key[i]) == 0)
		{
			targetName = i;
		}
	}

	if (triggerName == -1 || count == -1 || targetType == -1 || targetName == -1)
	{
		printf("Trigger is missing resources\n");

		exit(1);
	}

	addGlobalTrigger(value[triggerName], atoi(value[count]), getTriggerTypeByName(value[targetType]), value[targetName]);
}

static void addGlobalTrigger(char *triggerName, int count, int targetType, char *targetName)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == FALSE)
		{
			trigger[i].inUse = TRUE;

			trigger[i].count = count;
			trigger[i].targetType = targetType;

			STRNCPY(trigger[i].triggerName, triggerName, sizeof(trigger[i].triggerName));
			STRNCPY(trigger[i].targetName, targetName, sizeof(trigger[i].targetName));

			printf("Added Global Trigger \"%s\" with count %d\n", trigger[i].triggerName, trigger[i].count);

			return;
		}
	}

	printf("No free slots to add global trigger \"%s\"\n", triggerName);

	exit(1);
}

void fireGlobalTrigger(char *name)
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

			printf("Trigger count is %d\n", trigger[i].count);

			if (trigger[i].count <= 0)
			{
				printf("Firing global trigger %s\n", trigger[i].triggerName);

				switch (trigger[i].targetType)
				{
					case UPDATE_OBJECTIVE:
						updateObjective(trigger[i].targetName);
					break;

					case ACTIVATE_ENTITY:
						activateEntitiesWithName(trigger[i].targetName, TRUE);
					break;

					case RUN_SCRIPT:
						loadScript(trigger[i].targetName);

						readNextScriptLine();
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

void writeGlobalTriggersToFile(FILE *fp)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE GLOBAL_TRIGGER\n");
			fprintf(fp, "TRIGGER_NAME %s\n", trigger[i].triggerName);
			fprintf(fp, "TRIGGER_COUNT %d\n", trigger[i].count);
			fprintf(fp, "TRIGGER_TYPE %s\n", getTriggerTypeByID(trigger[i].targetType));
			fprintf(fp, "TRIGGER_TARGET %s\n", trigger[i].targetName);
			fprintf(fp, "}\n\n");
		}
	}
}
