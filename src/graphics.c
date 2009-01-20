#include "graphics.h"

SDL_Surface *loadImage(char *name)
{
	/* Load the image using SDL Image */

	char path[MAX_PATH_LENGTH];
	SDL_Surface *temp;
	SDL_Surface *image;

	sprintf(path, INSTALL_PATH"%s", name);

	temp = IMG_Load(path);

	if (temp == NULL)
	{
		printf("Failed to load image %s\n", path);

		exit(1);
	}

	/* Make the background transparent */

	SDL_SetColorKey(temp, (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(temp->format, TRANS_R, TRANS_G, TRANS_B));

	/* Convert the image to the screen's native format */

	image = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	if (image == NULL)
	{
		printf("Failed to convert image %s to native format\n", name);

		exit(1);
	}

	/* Return the processed image */

	return image;
}

void drawImage(SDL_Surface *image, int x, int y)
{
	SDL_Rect dest;

	/* Set the blitting rectangle to the size of the src image */

	dest.x = x;
	dest.y = y;
	dest.w = image->w;
	dest.h = image->h;

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(image, NULL, game.screen, &dest);
}

void drawFlippedImage(SDL_Surface *image, int destX, int destY)
{
	int *pixels, x, y, pixel, rx, ry;
	SDL_Rect dest;
	SDL_Surface *flipped;

	flipped = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, image->format->BitsPerPixel, image->format->Rmask, image->format->Gmask, image->format->Bmask, 0);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (x=0, rx=flipped->w-1;x<flipped->w;x++, rx--)
	{
		for (y=0, ry=flipped->h-1;y<flipped->h;y++, ry--)
		{
			pixels = (int *)image->pixels;

			pixel = pixels[(y * image->w) + x];

			pixels = (int *)flipped->pixels;

			pixels[(y * flipped->w) + rx] = pixel;
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	if (image->flags & SDL_SRCCOLORKEY)
	{
		SDL_SetColorKey(flipped, SDL_RLEACCEL | SDL_SRCCOLORKEY, image->format->colorkey);
	}

	/* Set the blitting rectangle to the size of the src image */

	dest.x = destX;
	dest.y = destY;
	dest.w = flipped->w;
	dest.h = flipped->h;

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(flipped, NULL, game.screen, &dest);

	SDL_FreeSurface(flipped);
}

void drawBox(int x, int y, int w, int h, int r, int g, int b)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_FillRect(game.screen, &rect, color);
}

void clearScreen(int r, int g, int b)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);

	SDL_FillRect(game.screen, NULL, color);
}

int isTransparent(SDL_Surface *image, int x, int y)
{
	int *pixels, pixel;
	unsigned char r, g, b;

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	pixels = (int *)image->pixels;

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	pixel = pixels[(y * image->w) + x];

	SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

	return (r == TRANS_R && g == TRANS_G && b == TRANS_B);
}
