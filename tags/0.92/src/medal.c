/*
Copyright (C) 2009-2011 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "headers.h"

#include "system/load_save.h"
#include "system/error.h"
#include "hud.h"
#include "medal.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "input.h"
#include "credits.h"
#include "system/pak.h"

extern Game game;

static NetworkMedal networkMedal;
static Medal *medal;
static Message messageHead;
static int medalCount, awardedMedalIndex;

static void addMedalToQueue(char *);
static void getNextMedalFromQueue(void);
static void loadMedals(void);
static int getOnlineMedals(void *);

void initMedals()
{
	messageHead.next = NULL;

	networkMedal.medalMessage.text[0] = '\0';
	
	loadMedals();
	
	loadObtainedMedals();
	
	networkMedal.thread = SDL_CreateThread(&getOnlineMedals, NULL);
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
	if (strlen(networkMedal.medalMessage.text) == 0)
	{
		awardedMedalIndex = -1;
		
		getNextMedalFromQueue();
		
		networkMedal.thinkTime = 60;
	}
	
	else
	{
		networkMedal.thinkTime--;
		
		if (networkMedal.thinkTime <= 0)
		{
			networkMedal.thinkTime = 0;
			
			showMedal(medal[awardedMedalIndex].medalType, medal[awardedMedalIndex].description);
			
			networkMedal.thinkTime = 60;
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
	int i;
	Message *head = messageHead.next;

	if (head != NULL)
	{
		for (i=0;i<medalCount;i++)
		{
			if (medal[i].obtained == FALSE && strcmpignorecase(medal[i].code, head->text) == 0)
			{
				STRNCPY(networkMedal.medalMessage.text, head->text, sizeof(networkMedal.medalMessage.text));
				
				medal[i].obtained = TRUE;
				
				awardedMedalIndex = i;
				
				break;
			}
		}

		messageHead.next = head->next;

		free(head);
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

	networkMedal.medalMessage.text[0] = '\0';

	SDL_KillThread(networkMedal.thread);
	
	saveObtainedMedals();
	
	if (medal != NULL)
	{
		free(medal);
		
		medal = NULL;
	}
}

void medalProcessingFinished()
{
	networkMedal.medalMessage.text[0] = '\0';
}

static void loadMedals()
{
	int i;
	char *line, *medalType, *hidden, *code, *description, *savePtr1, *savePtr2;
	unsigned char *buffer;

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
		
		else
		{
			medal[i].medalType = 3;
		}
		
		medal[i].hidden = strcmpignorecase(hidden, "Y") == 0 ? TRUE : FALSE;
		
		STRNCPY(medal[i].description, description, sizeof(medal[i].description));
		
		medal[i].obtained = FALSE;
		
		i++;
		
		line = strtok_r(NULL, "\n", &savePtr1);
	}
}

Medal *getMedals()
{
	return medal;
}

int getMedalCount()
{
	return medalCount;
}

void resetObtainedMedals()
{
	int i;
	
	for (i=0;i<medalCount;i++)
	{
		medal[i].obtained = FALSE;
	}
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

static int getOnlineMedals(void *data)
{
	char in[MAX_LINE_LENGTH], out[MAX_LINE_LENGTH], *savePtr;
	char *token;
	int len;
	TCPsocket socket;
	
	if (getPrivateKey(networkMedal.privateKey) == FALSE)
	{
		return 0;
	}
	
	if (SDLNet_ResolveHost(&networkMedal.ip, MEDAL_SERVER_HOST, MEDAL_SERVER_PORT) == -1)
	{
		printf("Could not connect to medal server: %s\n", SDLNet_GetError());

		return 1;
	}

	socket = SDLNet_TCP_Open(&networkMedal.ip);

	if (socket == NULL)
	{
		printf("Failed to create socket: %s\n", SDLNet_GetError());

		return 1;
	}

	snprintf(out, MAX_LINE_LENGTH, "GET /getMedals/%s HTTP/1.1\nHost: %s\nUser-Agent:LOE%.2f-%d\n\n", networkMedal.privateKey, MEDAL_SERVER_HOST, VERSION, RELEASE);

	len = strlen(out) + 1;

	if (SDLNet_TCP_Send(socket, (void*)out, len) < len)
	{
		printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		
		SDLNet_TCP_Close(socket);

		return 1;
	}

	SDLNet_TCP_Recv(socket, in, MAX_LINE_LENGTH - 1);

	token = strtok_r(in, "\n", &savePtr);

	while (token != NULL)
	{
		if (strstr(token, "LOE_"))
		{
			STRNCPY(out, token + 4, sizeof(out));
			
			setObtainedMedal(out);
		}
		
		token = strtok_r(NULL, "\n", &savePtr);
	}

	SDLNet_TCP_Close(socket);

	return 0;
}
