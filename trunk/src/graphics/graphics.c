#include "../headers.h"

#include "../map.h"
#include "../collisions.h"
#include "../system/pak.h"

extern Game game;

static void drawImageWhite(SDL_Surface *, int, int);

SDL_Surface *loadImage(char *name)
{
	/* Load the image using SDL Image */

	char path[MAX_PATH_LENGTH];
	SDL_Surface *temp;
	SDL_Surface *image;

	snprintf(path, sizeof(path), INSTALL_PATH"%s", name);

	temp = IMG_Load(path);
	
	/*temp = loadImageFromPak(name);*/

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

void drawImage(SDL_Surface *image, int x, int y, int white)
{
	SDL_Rect dest;

	if (white == TRUE)
	{
		drawImageWhite(image, x, y);

		return;
	}

	/* Set the blitting rectangle to the size of the source image */

	dest.x = game.offsetX + x;
	dest.y = game.offsetY + y;
	dest.w = image->w;
	dest.h = image->h;

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(image, NULL, game.screen, &dest);
}

void drawClippedImage(SDL_Surface *image, int srcX, int srcY, int destX, int destY, int width, int height)
{
	SDL_Rect src, dest;

	src.x = srcX;
	src.y = srcY;
	src.w = width;
	src.h = height;

	dest.x = game.offsetX + destX;
	dest.y = game.offsetY + destY;
	dest.w = width;
	dest.h = height;

	SDL_BlitSurface(image, &src, game.screen, &dest);
}

void drawFlippedImage(SDL_Surface *image, int destX, int destY, int white)
{
	unsigned char r, g, b;
	int *pixels, x, y, pixel, rx, ry;
	int color = SDL_MapRGB(game.screen->format, 255, 255, 255);
	SDL_Rect dest;
	SDL_Surface *flipped, *temp;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, image->format->BitsPerPixel, image->format->Rmask, image->format->Gmask, image->format->Bmask, 0);

	flipped = SDL_DisplayFormat(temp);

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

			if (white == TRUE)
			{
				SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

				pixels[(y * flipped->w) + rx] = (r == TRANS_R && g == TRANS_G && b == TRANS_B) ? pixel : color;
			}

			else
			{
				pixels[(y * flipped->w) + rx] = pixel;
			}
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

	dest.x = game.offsetX + destX;
	dest.y = game.offsetY + destY;
	dest.w = flipped->w;
	dest.h = flipped->h;

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(flipped, NULL, game.screen, &dest);

	SDL_FreeSurface(flipped);

	SDL_FreeSurface(temp);
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

void drawBoxToMap(int x, int y, int w, int h, int r, int g, int b)
{
	int color;
	SDL_Rect rect;

	rect.x = x - getMapStartX();
	rect.y = y - getMapStartY();
	rect.w = w;
	rect.h = h;

	color = SDL_MapRGB(game.screen->format, r, g, b);

	if (collision(rect.x, rect.y, rect.w, rect.h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
	{
		SDL_FillRect(game.screen, &rect, color);
	}
}

void drawCircle(int x, int y, int radius, int r, int g, int b)
{
	int y1, y2;
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;

	for (y1=-radius,y2=radius;y1;++y1,--y2)
	{
		int xr = (int)(sqrt(radius * radius - y1 * y1) + 0.5);

		rect.x = x - xr;
		rect.y = y + y1;
		rect.w = 2 * xr;
		rect.h = 1;

		SDL_FillRect(game.screen, &rect, color);

		rect.y = y + y2;
		rect.h = 1;

		SDL_FillRect(game.screen, &rect, color);
	}

	rect.x = x - radius;
	rect.y = y;
	rect.w = 2 * radius;
	rect.h = 1;

	SDL_FillRect(game.screen, &rect, color);
}

void drawCircleFromSurface(int x, int y, int radius)
{
	int y1, y2;
	SDL_Rect src, dest;
	SDL_Surface *temp;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, game.screen->w, game.screen->h, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	game.tempSurface = SDL_DisplayFormat(temp);

	SDL_BlitSurface(game.screen, NULL, game.tempSurface, NULL);

	SDL_FillRect(game.screen, NULL, 0);

	for (y1=-radius,y2=radius;y1;++y1,--y2)
	{
		int xr = (int)(sqrt(radius * radius - y1 * y1) + 0.5);

		src.x = x - xr;
		src.y = y + y1;
		src.w = 2 * xr;
		src.h = 1;

		if (src.x < 0)
		{
			src.x = 0;
		}

		if (src.y < 0)
		{
			src.y = 0;
		}

		if (src.x + src.w > SCREEN_WIDTH)
		{
			src.w = SCREEN_WIDTH - src.x;
		}

		if (src.y + src.h > SCREEN_HEIGHT)
		{
			src.h = SCREEN_HEIGHT - src.y;
		}

		dest.x = x - xr;
		dest.y = y + y1;
		dest.w = 2 * xr;
		dest.h = 1;

		if (dest.x < 0)
		{
			dest.x = 0;
		}

		if (dest.y < 0)
		{
			dest.y = 0;
		}

		if (dest.x + dest.w > SCREEN_WIDTH)
		{
			dest.w = SCREEN_WIDTH - dest.x;
		}

		if (dest.y + dest.h > SCREEN_HEIGHT)
		{
			dest.h = SCREEN_HEIGHT - dest.y;
		}

		SDL_BlitSurface(game.tempSurface, &src, game.screen, &dest);

		src.y = y + y2;
		src.h = 1;

		if (src.x < 0)
		{
			src.x = 0;
		}

		if (src.y < 0)
		{
			src.y = 0;
		}

		if (src.x + src.w > SCREEN_WIDTH)
		{
			src.w = SCREEN_WIDTH - src.x;
		}

		if (src.y + src.h > SCREEN_HEIGHT)
		{
			src.h = SCREEN_HEIGHT - src.y;
		}

		dest.y = y + y2;
		dest.h = 1;

		if (dest.x < 0)
		{
			dest.x = 0;
		}

		if (dest.y < 0)
		{
			dest.y = 0;
		}

		if (dest.x + dest.w > SCREEN_WIDTH)
		{
			dest.w = SCREEN_WIDTH - dest.x;
		}

		if (dest.y + dest.h > SCREEN_HEIGHT)
		{
			dest.h = SCREEN_HEIGHT - dest.y;
		}

		SDL_BlitSurface(game.tempSurface, &src, game.screen, &dest);
	}

	src.x = x - radius;
	src.y = y;
	src.w = 2 * radius;
	src.h = 1;

	if (src.x < 0)
	{
		src.x = 0;
	}

	if (src.y < 0)
	{
		src.y = 0;
	}

	if (src.x + src.w > SCREEN_WIDTH)
	{
		src.w = SCREEN_WIDTH - src.x;
	}

	if (src.y + src.h > SCREEN_HEIGHT)
	{
		src.h = SCREEN_HEIGHT - src.y;
	}

	dest.x = x - radius;
	dest.y = y;
	dest.w = 2 * radius;
	dest.h = 1;

	if (dest.x < 0)
	{
		dest.x = 0;
	}

	if (dest.y < 0)
	{
		dest.y = 0;
	}

	if (dest.x + dest.w > SCREEN_WIDTH)
	{
		dest.w = SCREEN_WIDTH - dest.x;
	}

	if (dest.y + dest.h > SCREEN_HEIGHT)
	{
		dest.h = SCREEN_HEIGHT - dest.y;
	}

	SDL_BlitSurface(game.tempSurface, &src, game.screen, &dest);

	SDL_FreeSurface(game.tempSurface);

	SDL_FreeSurface(temp);
}

SDL_Surface *addBorder(SDL_Surface *surface, int r, int g, int b, int br, int bg, int bb)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;
	SDL_Surface *temp, *newSurface;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w + 10, surface->h + 10, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, 0);

	newSurface = SDL_DisplayFormat(temp);
	
	SDL_FillRect(newSurface, NULL, SDL_MapRGB(game.screen->format, br, bg, bb));

	rect.x = 5;
	rect.y = 5;
	rect.w = surface->w;
	rect.h = surface->h;

	SDL_BlitSurface(surface, NULL, newSurface, &rect);

	/* Top */

	rect.x = 0;
	rect.y = 0;
	rect.w = newSurface->w;
	rect.h = 1;

	SDL_FillRect(newSurface, &rect, color);

	/* Left */

	rect.x = 0;
	rect.y = 0;
	rect.w = 1;
	rect.h = newSurface->h;

	SDL_FillRect(newSurface, &rect, color);

	/* Right */

	rect.x = newSurface->w - 1;
	rect.y = 0;
	rect.w = 1;
	rect.h = newSurface->h;

	SDL_FillRect(newSurface, &rect, color);

	/* Bottom */

	rect.x = 0;
	rect.y = newSurface->h - 1;
	rect.w = newSurface->w;
	rect.h = 1;

	SDL_FillRect(newSurface, &rect, color);

	SDL_FreeSurface(surface);

	SDL_FreeSurface(temp);

	return newSurface;
}

void clearScreen(int r, int g, int b)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);

	SDL_FillRect(game.screen, NULL, color);
}

static void drawImageWhite(SDL_Surface *image, int destX, int destY)
{
	unsigned char r, g, b;
	int *pixels, x, y, pixel;
	int color = SDL_MapRGB(game.screen->format, 255, 255, 255);
	SDL_Rect dest;
	SDL_Surface *flipped, *temp;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, image->format->BitsPerPixel, image->format->Rmask, image->format->Gmask, image->format->Bmask, 0);

	flipped = SDL_DisplayFormat(temp);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (x=0;x<image->w;x++)
	{
		for (y=0;y<image->h;y++)
		{
			pixels = (int *)image->pixels;

			pixel = pixels[(y * image->w) + x];

			SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

			pixels = (int *)flipped->pixels;

			pixels[(y * flipped->w) + x] = (r == TRANS_R && g == TRANS_G && b == TRANS_B) ? pixel : color;
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

	dest.x = game.offsetX + destX;
	dest.y = game.offsetY + destY;
	dest.w = flipped->w;
	dest.h = flipped->h;

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(flipped, NULL, game.screen, &dest);

	SDL_FreeSurface(flipped);

	SDL_FreeSurface(temp);
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
