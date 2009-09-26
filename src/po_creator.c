#include "headers.h"

static int textAlreadyAdded(char *text);

static char added[4096][MAX_LINE_LENGTH];
static int poIndex;

int main(int argc, char *argv[])
{
	DIR *dirp;
	FILE *fp;
	struct dirent *dfile;
	char filename[MAX_FILE_LENGTH], line[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], value2[MAX_LINE_LENGTH];
	char *token;

	memset(added, 0, 4096 * MAX_LINE_LENGTH);
	
	value[0] = '\0';
	value2[0] = '\0';

	poIndex = 0;

	/* Scripts */
	/*
	dirp = opendir("data/scripts");

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/scripts/%s", dfile->d_name);

		fp = fopen(filename, "rb");

		while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (line[strlen(line) - 1] == '\r')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (strstr(line, "TALK") != NULL)
			{
				token = strtok(line, " ");

				token = strtok(NULL, " ");

				token = strtok(NULL, "\0");

				if (textAlreadyAdded(token) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
				}
			}
			
			else if (strstr(line, "ADD ENTITY ITEM") != NULL)
			{
				sscanf(line, "%*s %*s %*s %*s \"%[^\"]\"", value);

				if (textAlreadyAdded(value) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", value);
				}
			}
			
			else if (strstr(line, "ADD OBJECTIVE") != NULL)
			{
				sscanf(line, "%*s %*s \"%[^\"]\"", value);

				if (textAlreadyAdded(value) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", value);
				}
			}

			else if (strstr(line, "ADD TRIGGER") != NULL && strstr(line, "UPDATE_OBJECTIVE") != NULL)
			{
				sscanf(line, "%*s %*s \"%[^\"]\" %*d %*s \"%[^\"]\"", value, value2);
				
				if (textAlreadyAdded(value) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", value);
				}
				
				if (textAlreadyAdded(value2) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", value2);
				}
			}
		}

		fclose(fp);
	}

	closedir(dirp);
	*/
	dirp = opendir("data/props/item");

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/props/item/%s", dfile->d_name);

		fp = fopen(filename, "rb");

		while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (line[strlen(line) - 1] == '\r')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (strstr(line, "DESCRIPTION") != NULL)
			{
				token = strtok(line, " ");

				token = strtok(NULL, "\0");

				if (textAlreadyAdded(token) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
				}
			}
			
			else if (strstr(line, "OBJECTIVE_NAME") != NULL && strstr(token, "/"))
			{
				token = strtok(line, " ");

				token = strtok(NULL, "\0");

				if (textAlreadyAdded(token) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
				}
			}
		}

		fclose(fp);
	}

	closedir(dirp);
	
	dirp = opendir("data/props/weapon");

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/props/weapon/%s", dfile->d_name);

		fp = fopen(filename, "rb");

		while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (line[strlen(line) - 1] == '\r')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (strstr(line, "DESCRIPTION") != NULL)
			{
				token = strtok(line, " ");

				token = strtok(NULL, "\0");

				if (textAlreadyAdded(token) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
				}
			}

			else if (strstr(line, "OBJECTIVE_NAME") != NULL)
			{
				token = strtok(line, " ");

				token = strtok(NULL, "\0");

				if (textAlreadyAdded(token) == FALSE)
				{
					printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
				}
			}
		}

		fclose(fp);
	}

	closedir(dirp);
	/*
	dirp = opendir("data/maps");

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/maps/%s", dfile->d_name);

		fp = fopen(filename, "rb");

		while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (line[strlen(line) - 1] == '\r')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (strstr(line, "OBJECTIVE_NAME") != NULL)
			{
				token = strtok(line, " ");

				token = strtok(NULL, "\0");

				if (token != NULL && strlen(token) != 0 && strstr(token, "/") == NULL && strstr(token, "-") == NULL && strstr(token, "_") == NULL)
				{
					if (textAlreadyAdded(token) == FALSE)
					{
						printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
					}
				}
			}

			else if (strstr(line, "REQUIRES") != NULL)
			{
				token = strtok(line, " ");

				token = strtok(NULL, "\0");

				if (token != NULL && strlen(token) != 0 && strstr(token, "/") == NULL && strstr(token, "-") == NULL && strstr(token, "_") == NULL)
				{
					if (textAlreadyAdded(token) == FALSE)
					{
						printf("msgid \"%s\"\nmsgstr \"\"\n\n", token);
					}
				}
			}
		}

		fclose(fp);
	}

	closedir(dirp);
	*/
	return 0;
}

static int textAlreadyAdded(char *text)
{
	int i;

	for (i=0;i<poIndex;i++)
	{
		if (strcmpignorecase(added[i], text) == 0)
		{
			return TRUE;
		}
	}

	STRNCPY(added[poIndex], text, MAX_LINE_LENGTH);

	poIndex++;

	return FALSE;
}
