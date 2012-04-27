/*
Copyright (C) 2009-2012 Parallel Realities

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

SDL_Surface *loadImage(char *);
void drawImage(SDL_Surface *, int, int, int, int);
void drawBox(SDL_Surface *, int, int, int, int, int, int, int);
void clearScreen(int, int, int);
int isTransparent(SDL_Surface *, int, int);
void drawCircle(int, int, int, int, int, int);
void drawCircleFromSurface(int, int, int);
void drawClippedImage(SDL_Surface *, int, int, int, int, int, int);
void drawBoxToMap(int, int, int, int, int, int, int);
SDL_Surface *addBorder(SDL_Surface *, int, int, int, int, int, int);
void drawLine(int, int, int, int, int, int, int);
SDL_Surface *createSurface(int, int);
void drawHitBox(int, int, int, int);
void putPixelToMap(int, int, int, int, int);
SDL_Surface *flipImage(SDL_Surface *);
EntityList *createPixelsFromSprite(Sprite *);
void drawColouredLine(int, int, int, int, Uint32, Uint32, Uint32);
int getColour(int r, int g, int b);
SDL_Surface *copyImage(SDL_Surface *, int, int, int, int);
void takeScreenshot(void);
void takeSingleScreenshot(char *);
void setScreenshotDir(char *);
