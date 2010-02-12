/*
Copyright (C) 2009-2010 Parallel Realities

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

static Medal medal;
static Message messageHead;

static void addMedalToQueue(char *);
static void connectToServer(void);
static int postMedal(void *);
static void getNextMedalFromQueue(void);

void initMedals()
{
	messageHead.next = NULL;

	medal.medalMessage.text[0] = '\0';

	connectToServer();
}

void addMedal(char *medalName)
{
	if (medal.connected == TRUE)
	{
		addMedalToQueue(medalName);
	}
}

void processMedals()
{
	if (medal.processing == FALSE)
	{
		if (strlen(medal.medalMessage.text) == 0)
		{
			getNextMedalFromQueue();
		}

		else
		{
			medal.processing = TRUE;

			medal.thread = SDL_CreateThread(&postMedal, NULL);
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

	msg = (Message *)malloc(sizeof(Message));

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
	Message *head = messageHead.next;

	if (head != NULL)
	{
		STRNCPY(medal.medalMessage.text, head->text, sizeof(medal.medalMessage.text));

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

	medal.medalMessage.text[0] = '\0';

	SDL_KillThread(medal.thread);
}

static int postMedal(void *data)
{
	char medalName[MAX_MESSAGE_LENGTH], in[MAX_LINE_LENGTH], out[MAX_LINE_LENGTH];
	char *token, *store;
	int i, len, response;
	TCPsocket socket;

	STRNCPY(medalName, medal.medalMessage.text, MAX_MESSAGE_LENGTH);

	for (i=0;i<strlen(medalName);i++)
	{
		if (medalName[i] == ' ' || medalName[i] == '#' || medalName[i] == ',')
		{
			medalName[i] = '_';
		}
	}

	medal.medalMessage.text[0] = '\0';

	printf("Attempting to post medal 'LOE_%s'\n", medalName);

	socket = SDLNet_TCP_Open(&medal.ip);

	if (socket == NULL)
	{
		printf("Failed to create socket: %s\n", SDLNet_GetError());

		medal.processing = FALSE;

		return 0;
	}

	snprintf(out, MAX_LINE_LENGTH, "GET /addMedal/%s/LOE_%s HTTP/1.1\nHost: %s\nUser-Agent:LOE%.2f-%d\n\n", medal.privateKey, medalName, MEDAL_SERVER_HOST, VERSION, RELEASE);

	len = strlen(out) + 1;

	if (SDLNet_TCP_Send(socket, (void*)out, len) < len)
	{
		printf("Medal sending failed: %s\n", SDLNet_GetError());

		printf("Disconnected\n");

		SDLNet_TCP_Close(socket);

		medal.processing = FALSE;

		return 0;
	}

	SDLNet_TCP_Recv(socket, in, MAX_LINE_LENGTH - 1);

	response = 0;

	token = strtok_r(in, "\n", &store);

	while (token != NULL)
	{
		if (strstr(token, "MEDAL_RESPONSE"))
		{
			sscanf(token, "%*s %d %[^\n\r]", &response, out);

			break;
		}

		token = strtok_r(NULL, "\n", &store);
	}

	printf("MedalServer Response: %d '%s'\n", response, out);

	SDLNet_TCP_Close(socket);

	SDL_Delay(3000);
	
	if (response > 0 && response < 4)
	{
		while (showMedal(response, out) == FALSE)
		{
			SDL_Delay(1000);
		}
	}

	return 1;
}

static void connectToServer()
{
	if (medal.connected == TRUE)
	{
		return;
	}

	if (getPrivateKey(medal.privateKey) == FALSE)
	{
		return;
	}

	printf("Trying to connect to %s:%d\n", MEDAL_SERVER_HOST, MEDAL_SERVER_PORT);

	if (SDLNet_ResolveHost(&medal.ip, MEDAL_SERVER_HOST, MEDAL_SERVER_PORT) == -1)
	{
		printf("Could not connect to medal server: %s\n", SDLNet_GetError());

		return;
	}

	printf("Connected %s to %s:%d\n", medal.privateKey, MEDAL_SERVER_HOST, MEDAL_SERVER_PORT);

	medal.connected = TRUE;
}

void medalProcessingFinished()
{
	medal.processing = FALSE;
}
