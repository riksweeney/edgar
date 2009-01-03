#include "properties.h"

extern void loadSpritesFromFile(char *);
extern void loadAnimationData(char *);
extern void setEntityAnimation(Entity *, int);

void loadProperties(char *name, Entity *e)
{
	int i, j, index;
	char path[255], line[MAX_LINE_LENGTH];
	FILE *fp;
	
	sprintf(path, INSTALL_PATH"data/props/%s.props", name);
	
	index = -1;
	
	strcpy(e->name, name);
	
	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		if (strcmp(properties[i].name, name) == 0)
		{
			index = i;
			
			break;
		}
	}
		
	if (index == -1)
	{
		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			if (strlen(properties[i].name) == 0)
			{
				printf("Loading properties for %s to slot %d\n", name, i);
				
				fp = fopen(path, "rb");
				
				if (fp == NULL)
				{
					printf("Failed to open properties file %s\n", path);
					
					exit(1);
				}
				
				strcpy(properties[i].name, name);
				
				j = 0;
				
				while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
				{
					if (j == MAX_PROPS_ENTRIES)
					{
						printf("Cannot add any more properities\n");
						
						exit(1);
					}
					
					if (line[0] == '#' || line[0] == '\n')
					{
						continue;
					}
					
					sscanf(line, "%s %s", properties[i].key[j], properties[i].value[j]);
					
					if (strcmp(properties[i].key[j], "GFX_FILE") == 0)
					{
						loadSpritesFromFile(properties[i].value[j]);
					}
					
					if (strcmp(properties[i].key[j], "ANIM_FILE") == 0)
					{
						loadAnimationData(properties[i].value[j]);
					}
					
					j++;
				}
				
				fclose(fp);
				
				break;
			}
		}
		
		if (i == MAX_PROPS_FILES)
		{
			printf("No free slots for properties file %s\n", name);
			
			exit(1);
		}
	}
	
	index = 0;
	
	for (j=0;j<MAX_PROPS_ENTRIES;j++)
	{
		if (strcmp(properties[i].key[j], "HEALTH") == 0)
		{
			e->health = atoi(properties[i].value[j]);
		}
		
		else if (strcmp(properties[i].key[j], "TYPE") == 0)
		{
			e->type = atoi(properties[i].value[j]);
		}
		
		else if (strcmp(properties[i].key[j], "STAND") == 0)
		{
			e->animation[STAND] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "WALK") == 0)
		{
			e->animation[WALK] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "JUMP") == 0)
		{
			e->animation[JUMP] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "ATTACK_1") == 0)
		{
			e->animation[ATTACK_1] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "ATTACK_2") == 0)
		{
			e->animation[ATTACK_2] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "ATTACK_3") == 0)
		{
			e->animation[ATTACK_3] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "ATTACK_4") == 0)
		{
			e->animation[ATTACK_4] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "ATTACK_5") == 0)
		{
			e->animation[ATTACK_5] = atoi(properties[i].value[j]);
			
			index++;
		}
		
		else if (strcmp(properties[i].key[j], "DIE") == 0)
		{
			e->animation[DIE] = atoi(properties[i].value[j]);
			
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
