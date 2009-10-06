#include "headers.h"

static int textAlreadyAdded(char *text);

static char added[4096][MAX_LINE_LENGTH];
static int poIndex;

int main(int argc, char *argv[])
{
	FILE *fp;
	char line[MAX_LINE_LENGTH];

	memset(added, 0, 4096 * MAX_LINE_LENGTH);

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
