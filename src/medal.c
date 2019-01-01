/*
Copyright (C) 2009-2019 Parallel Realities

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

#include "credits.h"
#include "hud.h"
#include "medal.h"
#include "system/error.h"
#include "system/load_save.h"
#include "system/pak.h"

extern Game game;

static MedalQueue medalQueue;
static Medal *medal;
static Message messageHead;
static int medalCount, awardedMedalIndex;

static void addMedalToQueue(char *);
static void getNextMedalFromQueue(void);
static void loadMedals(void);

void initMedals()
{
	messageHead.next = NULL;

	medalQueue.medalMessage.text[0] = '\0';

	loadMedals();

	loadObtainedMedals();
}

void addMedal(char *medalName)
{
	if (game.cheating == FALSE)
	{
		addMedalToQueue(medalName);
	}
}

void processMedals()
{
	if (strlen(medalQueue.medalMessage.text) == 0)
	{
		awardedMedalIndex = -1;

		getNextMedalFromQueue();

		medalQueue.thinkTime = 60;
	}

	else
	{
		medalQueue.thinkTime--;

		if (medalQueue.thinkTime <= 0)
		{
			medalQueue.thinkTime = 0;

			showMedal(medal[awardedMedalIndex].medalType, medal[awardedMedalIndex].description);

			medalQueue.thinkTime = 60;
		}
	}
}

static void addMedalToQueue(char *text)
{
	Message *head, *msg;

	head = &messageHead;

	while (head->next != NULL)
	{
		if (strcmpignorecase(text, head->text) == 0)
		{
			return;
		}

		head = head->next;
	}

	msg = malloc(sizeof(Message));

	if (msg == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for Medal queue", (int)sizeof(Message));
	}

	STRNCPY(msg->text, text, sizeof(messageHead.text));

	msg->next = NULL;

	head->next = msg;
}

static void getNextMedalFromQueue()
{
	int i, medalsAwarded;
	Message *head = messageHead.next;

	medalsAwarded = 0;

	if (head != NULL)
	{
		for (i=0;i<medalCount;i++)
		{
			if (medal[i].obtained == FALSE && strcmpignorecase(medal[i].code, head->text) == 0)
			{
				STRNCPY(medalQueue.medalMessage.text, head->text, sizeof(medalQueue.medalMessage.text));

				medal[i].obtained = TRUE;

				awardedMedalIndex = i;
			}

			if (medal[i].obtained == TRUE)
			{
				medalsAwarded++;
			}
		}

		messageHead.next = head->next;

		free(head);
	}

	if (medalsAwarded == medalCount - 1)
	{
		addMedal("all_medals");
	}
}

void freeMedalQueue()
{
	Message *p, *q;

	for (p=messageHead.next;p!=NULL;p=q)
	{
		q = p->next;

		free(p);
	}

	messageHead.next = NULL;

	medalQueue.medalMessage.text[0] = '\0';

	if (medal != NULL)
	{
		saveObtainedMedals();

		free(medal);

		medal = NULL;
	}
}

void medalProcessingFinished()
{
	medalQueue.medalMessage.text[0] = '\0';
}

static void loadMedals()
{
	int i;
	char *line, *medalType, *hidden, *code, *description, *savePtr1, *savePtr2;
	unsigned char *buffer;

	savePtr1 = NULL;

	savePtr2 = NULL;

	buffer = loadFileFromPak("data/medals.dat");

	medalCount = countTokens((char *)buffer, "\n");

	medal = malloc(medalCount * sizeof(Medal));

	if (medal == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for Medals", medalCount * (int)sizeof(Medal));
	}

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	i = 0;

	while (line != NULL)
	{
		code = strtok_r(line, " ", &savePtr2);

		medalType = strtok_r(NULL, " ", &savePtr2);

		hidden = strtok_r(NULL, " ", &savePtr2);

		description = strtok_r(NULL, "\0", &savePtr2);

		STRNCPY(medal[i].code, code, sizeof(medal[i].code));

		if (strcmpignorecase(medalType, "B") == 0)
		{
			medal[i].medalType = 0;
		}

		else if (strcmpignorecase(medalType, "S") == 0)
		{
			medal[i].medalType = 1;
		}

		else if (strcmpignorecase(medalType, "G") == 0)
		{
			medal[i].medalType = 2;
		}

		else if (strcmpignorecase(medalType, "R") == 0)
		{
			medal[i].medalType = 3;
		}

		else
		{
			showErrorAndExit("Unknown Medal type %s", medalType);
		}

		medal[i].hidden = strcmpignorecase(hidden, "Y") == 0 ? TRUE : FALSE;

		STRNCPY(medal[i].description, description, sizeof(medal[i].description));

		medal[i].obtained = FALSE;

		i++;

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	free(buffer);
}

Medal *getMedals()
{
	return medal;
}

int getMedalCount()
{
	return medalCount;
}

void setObtainedMedal(char *medalCode)
{
	int i;

	for (i=0;i<medalCount;i++)
	{
		if (strcmpignorecase(medal[i].code, medalCode) == 0)
		{
			medal[i].obtained = TRUE;

			break;
		}
	}
}
