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
#include "graphics/font.h"
#include "graphics/graphics.h"

extern Game game;

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
	char medalName[MAX_MESSAGE_LENGTH], in[MAX_LINE_LENGTH], out[2][MAX_LINE_LENGTH];
	char *token, *store;
	int i, len, response[2];
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

	snprintf(out[0], MAX_LINE_LENGTH, "GET /addMedal/%s/LOE_%s HTTP/1.1\nHost: %s\nUser-Agent:LOE%.2f-%d\n\n", medal.privateKey, medalName, MEDAL_SERVER_HOST, VERSION, RELEASE);

	len = strlen(out[0]) + 1;

	if (SDLNet_TCP_Send(socket, (void*)out[0], len) < len)
	{
		printf("Medal sending failed: %s\n", SDLNet_GetError());

		printf("Disconnected\n");

		SDLNet_TCP_Close(socket);

		medal.processing = FALSE;

		return 0;
	}

	SDLNet_TCP_Recv(socket, in, MAX_LINE_LENGTH - 1);

	response[0] = response[1] = 0;

	token = strtok_r(in, "\n", &store);

	i = 0;

	while (token != NULL)
	{
		if (strstr(token, "MEDAL_RESPONSE"))
		{
			sscanf(token, "%*s %d %[^\n\r]", &response[i], out[i]);

			i++;

			if (i == 2)
			{
				break;
			}
		}

		token = strtok_r(NULL, "\n", &store);
	}

	SDLNet_TCP_Close(socket);

	printf("MedalServer Response: %d '%s'\n", response[1], out[1]);

	if (response[1] > 0 && response[1] <= 4)
	{
		SDL_Delay(3000);

		while (showMedal(response[1], out[1]) == FALSE)
		{
			SDL_Delay(1000);
		}
	}

	printf("MedalServer Response: %d '%s'\n", response[0], out[0]);

	if (response[0] > 0 && response[0] <= 4)
	{
		SDL_Delay(3000);

		while (showMedal(response[0], out[0]) == FALSE)
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
		medal.privateKeyFound = FALSE;

		return;
	}

	medal.privateKeyFound = TRUE;

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

void showMedalScreen()
{
	int i, h, y;
	SDL_Rect src, dest;
	SDL_Surface *title, *message, *message2;
	char text[MAX_MESSAGE_LENGTH];

	if (medal.privateKeyFound == TRUE)
	{
		if (medal.connected == TRUE)
		{
			return;
		}

		else
		{
			snprintf(text, MAX_MESSAGE_LENGTH, _("Could not connect to Medal Server at %s:%d"), MEDAL_SERVER_HOST, MEDAL_SERVER_PORT);

			message = generateTextSurface(text, game.font, 220, 0, 0, 0, 0, 0);
		}
	}

	else
	{
		snprintf(text, MAX_MESSAGE_LENGTH, _("Could not find Medal Key"));

		message = generateTextSurface(text, game.font, 220, 0, 0, 0, 0, 0);
	}

	title = generateTextSurface(_("The Legend of Edgar"), game.font, 0, 220, 0, 0, 0, 0);

	message2 = generateTextSurface(_("You will not be able to earn Medals for this game"), game.font, 220, 0, 0, 0, 0, 0);

	if (title == NULL || message == NULL || message2 == NULL)
	{
		return;
	}

	if (game.tempSurface != NULL)
	{
		SDL_FreeSurface(game.tempSurface);

		game.tempSurface = NULL;
	}

	game.tempSurface = createSurface(SCREEN_WIDTH, SCREEN_HEIGHT);

	drawBox(game.tempSurface, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

	h = title->h + message->h + message2->h + 90;

	y = (SCREEN_HEIGHT - h) / 2;

	src.x = 0;
	src.y = 0;
	src.w = title->w;
	src.h = title->h;

	dest.x = (SCREEN_WIDTH - title->w) / 2;
	dest.y = y;
	dest.w = title->w;
	dest.h = title->h;

	SDL_BlitSurface(title, &src, game.tempSurface, &dest);

	y += message->h + 45;

	src.x = 0;
	src.y = 0;
	src.w = message->w;
	src.h = message->h;

	dest.x = (SCREEN_WIDTH - message->w) / 2;
	dest.y = y;
	dest.w = message->w;
	dest.h = message->h;

	SDL_BlitSurface(message, &src, game.tempSurface, &dest);

	y += message2->h + 45;

	src.x = 0;
	src.y = 0;
	src.w = message2->w;
	src.h = message2->h;

	dest.x = (SCREEN_WIDTH - message2->w) / 2;
	dest.y = y;
	dest.w = message2->w;
	dest.h = message2->h;

	SDL_BlitSurface(message2, &src, game.tempSurface, &dest);

	SDL_FreeSurface(title);
	SDL_FreeSurface(message);
	SDL_FreeSurface(message2);

	for (i=0;i<120;i++)
	{
		clearScreen(0, 0, 0);

		SDL_BlitSurface(game.tempSurface, NULL, game.screen, NULL);

		/* Swap the buffers */

		SDL_Flip(game.screen);

		/* Sleep briefly */

		SDL_Delay(16);
	}
}
