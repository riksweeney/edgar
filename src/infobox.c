static Message message;

void setInfoBoxMessage(char *text)
{
	if (strcmpignorecase(text, message.text) != 0)
	{
		strcpy(message.text, text);
		
		if (message.surface != NULL)
		{
			SDL_FreeSurface(message.surface);
			
			message.surface = NULL;
		}
		
		message.surface = generateTextSurface(message.text, game.font);
	}
	
	message.thinkTime = 120;
}

void doInfoBox()
{
	message.thinkTime--;
	
	if (message.thinkTime <= 0)
	{
		if (message.surface != NULL)
		{
			SDL_FreeSurface(message.surface);
			
			message.surface = NULL;
		}
	}
}

void drawInfoBox()
{
	if (message.surface != NULL)
	{
		drawImage(message.surface, (SCREEN_WIDTH - message.surface->w) / 2, 400);
	}
}
