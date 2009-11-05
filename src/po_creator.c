#include "headers.h"

static int textAlreadyAdded(char *text);

static char added[8192][MAX_LINE_LENGTH];
static int poIndex;

int main(int argc, char *argv[])
{
	FILE *fp;
	char line[MAX_LINE_LENGTH], filename[MAX_LINE_LENGTH], *token;
	DIR *dirp, *dirp2;
	struct dirent *dfile;

	memset(added, 0, 8192 * MAX_LINE_LENGTH);

	poIndex = 0;

	fp = fopen("locale/messages.pot", "rb");

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

		if (strstr(line, "msgid") != NULL)
		{
			if (textAlreadyAdded(line) == FALSE)
			{
				printf("%s\nmsgstr \"\"\n\n", line);
			}
		}
	}

	fclose(fp);

	fp = fopen("messages.po", "rb");

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

		if (strstr(line, "msgid") != NULL)
		{
			if (textAlreadyAdded(line) == FALSE)
			{
				printf("%s\nmsgstr \"\"\n\n", line);
			}
		}
	}

	fclose(fp);

	dirp = opendir("data/scripts");

	if (dirp == NULL)
	{
		printf("Failed to open data/scripts\n");

		exit(1);
	}

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/scripts/%s", dfile->d_name);

		dirp2 = opendir(filename);

		if (dirp2)
		{
			closedir(dirp2);

			continue;
		}

		else
		{
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
					
					snprintf(line, sizeof(line), "msgid \"%s\"", token);

					if (textAlreadyAdded(line) == FALSE)
					{
						printf("%s\nmsgstr \"\"\n\n", line);
					}
				}
			}

			fclose(fp);
		}
	}

	closedir(dirp);

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
