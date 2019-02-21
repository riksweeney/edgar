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

Texture *loadImage(char *);
SDL_Surface *loadImageAsSurface(char *);
void destroyTexture(Texture *);
void drawImage(Texture *, int, int, int, int);
void drawImageToMap(Texture *, int, int, int, int);
void drawBox(int, int, int, int, int, int, int, int);
void drawBoxToSurface(SDL_Surface *, int, int, int, int, int, int, int);
Texture *copyScreen(void);
void clearScreen(int, int, int);
void drawCircle(int, int, int, int, int, int);
void drawCircleFromSurface(int, int, int);
void drawClippedImage(Texture *, int, int, int, int, int, int);
void drawBoxToMap(int, int, int, int, int, int, int);
Texture *addBorder(SDL_Surface *, int, int, int, int, int, int);
void drawLine(int, int, int, int, int, int, int);
SDL_Surface *createSurface(int, int, int);
Texture *convertSurfaceToTexture(SDL_Surface *, int);
Texture *createTexture(int, int, int, int, int);
Texture *createWritableTexture(int, int);
void drawHitBox(int, int, int, int);
void putPixelToMap(int, int, int, int, int);
Texture *convertImageToWhite(SDL_Surface *, int);
EntityList *createPixelsFromSprite(Sprite *);
void drawColouredLine(int, int, int, int, Colour, Colour, Colour);
void takeScreenshot(void);
void takeSingleScreenshot(char *);
void setScreenshotDir(char *);
