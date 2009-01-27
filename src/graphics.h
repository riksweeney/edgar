SDL_Surface *loadImage(char *);
void drawImage(SDL_Surface *, int, int);
void drawFlippedImage(SDL_Surface *, int, int);
void drawBox(int, int, int, int, int, int, int);
void clearScreen(int, int, int);
int isTransparent(SDL_Surface *, int, int);
