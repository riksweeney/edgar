#include "headers.h"

#include "animation.h"
#include "graphics.h"
#include "inventory.h"
#include "font.h"

extern Game game;

static SDL_Surface *itemBox;
static Message message[MAX_HUD_MESSAGES];

void initHud()
{
	itemBox = loadImage(INSTALL_PATH"gfx/hud/item_box.png");
}

void doHud()
{
	int i, j;

	if (message[0].thinkTime > 0)
	{
		message[0].thinkTime--;

		if (message[0].thinkTime <= 0)
		{
			message[0].inUse = FALSE;

			for (i=0;i<MAX_HUD_MESSAGES;i++)
			{
				if (message[i].inUse == FALSE)
				{
					for (j=i;j<MAX_HUD_MESSAGES;j++)
					{
						if (message[j].inUse == TRUE)
						{
							message[i] = message[j];

							message[j].inUse = FALSE;

							break;
						}
					}
				}
			}
		}
	}
}

void drawHud()
{
	drawSelectedInventoryItem((SCREEN_WIDTH - itemBox->w) / 2, 15, itemBox->w, itemBox->h);

	drawImage(itemBox, (SCREEN_WIDTH - itemBox->w) / 2, 15);

	if (message[0].inUse == TRUE)
	{
		switch (message[0].type)
		{
			case STANDARD_MESSAGE:
				drawString(message[0].text, 0, 400, game.font, 1, 0, 255, 255, 255);
			break;

			case GOOD_MESSAGE:
				drawString(message[0].text, 0, 400, game.font, 1, 0, 0, 200, 0);
			break;

			case BAD_MESSAGE:
				drawString(message[0].text, 0, 400, game.font, 1, 0, 200, 0, 0);
			break;
		}
	}
}

void freeHud()
{
	if (itemBox != NULL)
	{
		SDL_FreeSurface(itemBox);
	}
}

void addHudMessage(int type, char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	int i;
	va_list ap;

	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);


	for (i=0;i<MAX_HUD_MESSAGES;i++)
	{
		if (message[i].inUse == TRUE && strcmpignorecase(text, message[i].text) == 0)
		{
			message[i].thinkTime = 180;

			return;
		}

		else if (message[i].inUse == FALSE)
		{
			strcpy(message[i].text, text);

			message[i].thinkTime = 180;

			message[i].inUse = TRUE;

			message[i].type = type;

			printf("Setting message %d to %s\n", i, text);

			return;
		}
	}

	printf("Failed to set message to %s\n", text);
}

void freeHudMessages()
{
	memset(message, 0, sizeof(Message) * MAX_HUD_MESSAGES);
}
