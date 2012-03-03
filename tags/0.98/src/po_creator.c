/*
Copyright (C) 2009-2012 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "headers.h"

static int textAlreadyAdded(char *text);
static char *replaceString(char *, char *, char *);
static void recurseDirectory(char *, char *, int *);
static int checkExists(char *);

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

	fp = fopen("locale/edgar.pot", "rb");

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

				if (strstr(line, "AUTO_TALK") != NULL)
				{
					token = strtok(line, " ");

					token = strtok(NULL, " ");

					token = strtok(NULL, " ");

					token = strtok(NULL, "\0");

					snprintf(line, sizeof(line), "msgid \"%s\"", token);

					if (textAlreadyAdded(line) == FALSE)
					{
						printf("%s\nmsgstr \"\"\n\n", line);
					}
				}

				else if (strstr(line, "TALK") != NULL)
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

				else if (strstr(line, "ADD OBJECTIVE") != NULL)
				{
					sscanf(line, "%*s %*s \"%[^\"]\"", filename);

					snprintf(line, sizeof(line), "msgid \"%s\"", filename);

					if (textAlreadyAdded(line) == FALSE)
					{
						printf("%s\nmsgstr \"\"\n\n", line);
					}
				}

				else if (strstr(line, "SHOW_CONFIRM") != NULL)
				{
					token = strtok(line, " ");

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

	dirp = opendir("data/props/item");

	if (dirp == NULL)
	{
		printf("Failed to open data/props/item\n");

		exit(1);
	}

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/props/item/%s", dfile->d_name);

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

				if (strstr(line, "DESCRIPTION") != NULL)
				{
					token = strtok(line, " ");

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

	dirp = opendir("data/props/weapon");

	if (dirp == NULL)
	{
		printf("Failed to open data/props/weapon\n");

		exit(1);
	}

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "data/props/weapon/%s", dfile->d_name);

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

				if (strstr(line, "DESCRIPTION") != NULL)
				{
					token = strtok(line, " ");

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
	char *clean;

	if (strcmpignorecase("msgid \"\"", text) == 0)
	{
		return TRUE;
	}

	clean = replaceString(text, "msgid \"", "");
	clean[strlen(clean) - 1] = '\0';
	clean = replaceString(clean, "\\\"", "\"");

	if (checkExists(clean) == FALSE)
	{
		return TRUE;
	}

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

static int checkExists(char *string)
{
	int found = FALSE;

	string = replaceString(string, "msgid \"", "");

	recurseDirectory("src", string, &found);

	if (found == FALSE)
	{
		recurseDirectory("data", string, &found);
	}

	return found;
}

static char *replaceString(char *string, char *find, char *replace)
{
	static char buffer[MAX_LINE_LENGTH];
	char *p;

	p = strstr(string, find);

	if (p == NULL)
	{
		return string;
	}

	strncpy(buffer, string, p - string);

	buffer[p - string] = '\0';

	snprintf(buffer + (p - string), MAX_LINE_LENGTH, "%s%s", replace, p + strlen(find));

	return buffer;
}

static void recurseDirectory(char *dirName, char *searchString, int *found)
{
	DIR *dirp, *dirp2;
	struct dirent *dfile;
	FILE *infile;
	char filename[1024];
	int read;
	unsigned long fileSize;
	unsigned char *buffer;

	dirp = opendir(dirName);

	if (dirp == NULL)
	{
		printf("%s: Directory does not exist or is not accessable\n", dirName);

		exit(1);
	}

	while ((dfile = readdir(dirp)) && *found == FALSE)
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "%s/%s", dirName, dfile->d_name);

		dirp2 = opendir(filename);

		if (dirp2)
		{
			closedir(dirp2);

			recurseDirectory(filename, searchString, found);
		}

		else
		{
			infile = fopen(filename, "rb");

			if (!infile)
			{
				printf("Couldn't open %s for reading!\n", filename);

				closedir(dirp);

				exit(1);
			}

			fseek(infile, 0L, SEEK_END);

			fileSize = ftell(infile);

			if (fileSize == 0)
			{
				printf("%s is an empty file.\n", filename);

				exit(0);
			}

			buffer = malloc((fileSize + 1) * sizeof(unsigned char));

			if (buffer == NULL)
			{
				printf("Could not create buffer\n");

				exit(1);
			}

			fseek(infile, 0L, SEEK_SET);

			read = fread(buffer, fileSize, 1, infile);

			if (strstr((char *)buffer, searchString) != NULL)
			{
				*found = TRUE;
			}

			fclose(infile);

			free(buffer);
		}
	}

	closedir(dirp);
}
