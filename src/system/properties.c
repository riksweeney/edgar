#include "../headers.h"

#include "../graphics/animation.h"
#include "../graphics/sprites.h"
#include "../audio/audio.h"
#include "../system/properties.h"

static Properties properties[MAX_PROPS_FILES];

static void setFlags(Entity *, char *);

static char *ignoreProps[] = {"GFX_FILE", "ANIM_FILE", "NAME", NULL};
static Type type[] = {
					{PLAYER, "PLAYER"},
					{WEAPON, "WEAPON"},
					{ITEM, "ITEM"},
					{KEY_ITEM, "KEY_ITEM"},
					{ENEMY, "ENEMY"},
					{HEALTH, "HEALTH"},
					{SHIELD, "SHIELD"},
					{AUTO_LIFT, "AUTO_LIFT"},
					{MANUAL_LIFT, "MANUAL_LIFT"},
					{TARGET, "TARGET"},
					{SPAWNER, "SPAWNER"},
					{PRESSURE_PLATE, "PRESSURE_PLATE"},
					{MANUAL_DOOR, "MANUAL_DOOR"},
					{AUTO_DOOR, "AUTO_DOOR"},
					{WEAK_WALL, "WEAK_WALL"},
					{SWITCH, "SWITCH"},
					{LINE_DEF, "LINE_DEF"},
					{LEVEL_EXIT, "LEVEL_EXIT"},
					{SAVE_POINT, "SAVE_POINT"},
					{TEMP_ITEM, "TEMP_ITEM"}
					};
static int length = sizeof(type) / sizeof(Type);

void freeProperties()
{
	memset(properties, 0, sizeof(Properties) * MAX_PROPS_FILES);
}

void loadProperties(char *name, Entity *e)
{
	int i, j, index, animationIndex, graphicsIndex, sprites[256];
	char path[MAX_PATH_LENGTH], line[MAX_LINE_LENGTH];
	char *token;
	FILE *fp;

	snprintf(path, sizeof(path), INSTALL_PATH"data/props/%s.props", name);

	index = -1;

	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		if (strcmpignorecase(properties[i].name, name) == 0)
		{
			index = i;

			break;
		}
	}

	for (i=0;i<256;i++)
	{
		sprites[i] = -1;
	}

	animationIndex = graphicsIndex = -1;

	if (index == -1)
	{
		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			if (strlen(properties[i].name) == 0)
			{
				fp = fopen(path, "rb");

				if (fp == NULL)
				{
					printf("Failed to open properties file %s\n", path);

					exit(1);
				}

				STRNCPY(properties[i].name, name, sizeof(properties[i].name));

				j = 0;

				while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
				{
					if (j == MAX_PROPS_ENTRIES)
					{
						printf("Cannot add any more properities\n");

						exit(1);
					}

					if (line[strlen(line) - 1] == '\n')
					{
						line[strlen(line) - 1] = '\0';
					}

					if (line[0] == '#' || line[0] == '\n')
					{
						continue;
					}

					token = strtok(line, " ");

					STRNCPY(properties[i].key[j], token, sizeof(properties[i].key[j]));

					token = strtok(NULL, "\0");

					if (token != NULL)
					{
						STRNCPY(properties[i].value[j], token, sizeof(properties[i].value[j]));
					}

					else
					{
						printf("%s: %s is missing value\n", name, properties[i].key[j]);

						exit(1);
					}

					if (strcmpignorecase(properties[i].key[j], "GFX_FILE") == 0)
					{
						graphicsIndex = j;
					}

					else if (strcmpignorecase(properties[i].key[j], "ANIM_FILE") == 0)
					{
						animationIndex = j;
					}

					j++;
				}

				fclose(fp);

				break;
			}
		}

		if (graphicsIndex != -1 && animationIndex != -1)
		{
			loadSpritesFromFile(properties[i].value[graphicsIndex], sprites);

			loadAnimationData(properties[i].value[animationIndex], sprites, properties[i].animations);
		}

		else if (graphicsIndex == 0)
		{
			printf("No graphics file found for %s\n", name);

			exit(1);
		}

		else
		{
			printf("No animation file found for %s\n", name);

			exit(1);
		}

		if (i == MAX_PROPS_FILES)
		{
			printf("No free slots for properties file %s\n", name);

			exit(1);
		}
	}

	else
	{
		i = index;
	}

	if (e != NULL)
	{
		index = 0;

		STRNCPY(e->name, name, sizeof(e->name));

		for (j=0;j<MAX_PROPS_ENTRIES;j++)
		{
			setProperty(e, properties[i].key[j], properties[i].value[j]);
		}

		for (j=0;j<MAX_ANIMATION_TYPES;j++)
		{
			e->animation[j] = properties[i].animations[j];

			if (e->animation[j] != -1)
			{
				index++;
			}
		}

		if (index == 0)
		{
			printf("No animations defined for %s\n", name);

			exit(1);
		}

		e->currentAnim = -1;

		setEntityAnimation(e, 0);
	}
}

static void setFlags(Entity *e, char *flags)
{
	char *token, *temp;

	temp = (char *)malloc(strlen(flags) + 1);

	if (temp == NULL)
	{
		printf("Could not allocate a whole %d bytes for flags...\n", strlen(flags) + 1);

		exit(1);
	}

	STRNCPY(temp, flags, strlen(flags) + 1);

	token = strtok(temp, " |,");

	e->flags = 0;

	while (token != NULL)
	{
		if (strcmpignorecase(token, "ON_GROUND") == 0)
		{
			e->flags |= ON_GROUND;
		}

		else if (strcmpignorecase(token, "PUSHABLE") == 0)
		{
			e->flags |= PUSHABLE;
		}

		else if (strcmpignorecase(token, "HELPLESS") == 0)
		{
			e->flags |= HELPLESS;
		}

		else if (strcmpignorecase(token, "INVULNERABLE") == 0)
		{
			e->flags |= INVULNERABLE;
		}

		else if (strcmpignorecase(token, "FLY") == 0)
		{
			e->flags |= FLY;
		}

		else if (strcmpignorecase(token, "ALWAYS_ON_TOP") == 0)
		{
			e->flags |= ALWAYS_ON_TOP;
		}

		else if (strcmpignorecase(token, "NO_DRAW") == 0)
		{
			e->flags |= NO_DRAW;
		}

		else if (strcmpignorecase(token, "STACKABLE") == 0)
		{
			e->flags |= STACKABLE;
		}

		else if (strcmpignorecase(token, "OBSTACLE") == 0)
		{
			e->flags |= OBSTACLE;
		}

		else if (strcmpignorecase(token, "FLOATS") == 0)
		{
			e->flags |= FLOATS;
		}
		
		else if (strcmpignorecase(token, "NO_TOUCH") == 0)
		{
			e->flags |= NO_TOUCH;
		}

		else
		{
			printf("Ignoring flag value %s\n", token);
		}

		token = strtok(NULL, " |,");
	}

	free(temp);
}

void setProperty(Entity *e, char *name, char *value)
{
	int i = 0, found = 0;

	if (strlen(name) == 0)
	{
		return;
	}

	if (strcmpignorecase(name, "X") == 0)
	{
		e->x = atoi(value);
	}

	else if (strcmpignorecase(name, "Y") == 0)
	{
		e->y = atoi(value);
	}

	else if (strcmpignorecase(name, "START_X") == 0)
	{
		e->startX = atoi(value);
	}

	else if (strcmpignorecase(name, "START_Y") == 0)
	{
		e->startY = atoi(value);
	}

	else if (strcmpignorecase(name, "END_X") == 0)
	{
		e->endX = atoi(value);
	}

	else if (strcmpignorecase(name, "END_Y") == 0)
	{
		e->endY = atoi(value);
	}

	else if (strcmpignorecase(name, "OBJECTIVE_NAME") == 0)
	{
		STRNCPY(e->objectiveName, value, sizeof(e->objectiveName));
	}

	else if (strcmpignorecase(name, "REQUIRES") == 0)
	{
		STRNCPY(e->requires, value, sizeof(e->requires));
	}

	else if (strcmpignorecase(name, "THINKTIME") == 0)
	{
		e->thinkTime = atoi(value);

		if (e->maxThinkTime == 0)
		{
			e->maxThinkTime = e->thinkTime;
		}
	}

	else if (strcmpignorecase(name, "MAX_THINKTIME") == 0)
	{
		e->maxThinkTime = atoi(value);
	}

	else if (strcmpignorecase(name, "SPEED") == 0)
	{
		e->speed = atof(value);
	}

	else if (strcmpignorecase(name, "ACTIVE") == 0)
	{
		e->active = strcmpignorecase(value, "TRUE") == 0 ? TRUE : FALSE;
	}

	else if (strcmpignorecase(name, "SFX_FILE") == 0)
	{
		preCacheSounds(value);
	}

	else if (strcmpignorecase(name, "HEALTH") == 0)
	{
		e->health = atoi(value);
		
		if (e->maxHealth < e->health)
		{
			e->maxHealth = e->health;
		}
	}

	else if (strcmpignorecase(name, "MAX_HEALTH") == 0)
	{
		e->maxHealth = atoi(value);
	}

	else if (strcmpignorecase(name, "DAMAGE") == 0)
	{
		e->damage = atoi(value);
	}

	else if (strcmpignorecase(name, "THINKTIME") == 0)
	{
		e->thinkTime = atoi(value);
	}

	else if (strcmpignorecase(name, "SPEED") == 0)
	{
		e->speed = atof(value);
	}

	else if (strcmpignorecase(name, "WEIGHT") == 0)
	{
		e->weight = atof(value);
	}

	else if (strcmpignorecase(name, "FLAGS") == 0)
	{
		setFlags(e, value);
	}

	else if (strcmpignorecase(name, "TYPE") == 0)
	{
		e->type = getEntityTypeByName(value);
	}

	else if (strcmpignorecase(name, "FACE") == 0)
	{
		e->face = strcmpignorecase(value, "RIGHT") == 0 ? RIGHT : LEFT;
	}

	else
	{
		while (ignoreProps[i] != NULL)
		{
			if (strcmpignorecase(name, ignoreProps[i]) == 0)
			{
				found = TRUE;

				break;
			}

			i++;
		}

		if (found == FALSE)
		{
			printf("%s has unknown property value %s\n", e->name, name);
		}
	}
}

int getEntityTypeByName(char *name)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(name, type[i].name) == 0)
		{
			return type[i].id;
		}
	}

	printf("Unknown Entity Type %s\n", name);

	exit(1);
}

char *getEntityTypeByID(int id)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (id == type[i].id)
		{
			return type[i].name;
		}
	}

	printf("Unknown Entity ID %d\n", id);

	exit(1);
}
