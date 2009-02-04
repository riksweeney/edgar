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
			message[0].inUse = NOT_IN_USE;

			for (i=0;i<MAX_HUD_MESSAGES;i++)
			{
				if (message[i].inUse == NOT_IN_USE)
				{
					for (j=i;j<MAX_HUD_MESSAGES;j++)
					{
						if (message[j].inUse == IN_USE)
						{
							message[i] = message[j];

							message[j].inUse = NOT_IN_USE;

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

	if (message[0].inUse == IN_USE)
	{
		drawString(message[0].text, 0, 400, game.font, 1, 0);
	}
}

void freeHud()
{
	if (itemBox != NULL)
	{
		SDL_FreeSurface(itemBox);
	}
}

void addHudMessage(char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	int i;
	va_list ap;

	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);


	for (i=0;i<MAX_HUD_MESSAGES;i++)
	{
		if (message[i].inUse == IN_USE && strcmpignorecase(text, message[i].text) == 0)
		{
			return;
		}

		else if (message[i].inUse == NOT_IN_USE)
		{
			strcpy(message[i].text, text);

			message[i].thinkTime = 180;

			message[i].inUse = IN_USE;

			printf("Setting message %d to %s\n", i, text);

			return;
		}
	}

	printf("Failed to set message to %s\n", text);
}
