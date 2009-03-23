SDL_Surface *loadImage(char *);
void drawImage(SDL_Surface *, int, int, int);
void drawFlippedImage(SDL_Surface *, int, int, int);
void drawBox(int, int, int, int, int, int, int);
void clearScreen(int, int, int);
int isTransparent(SDL_Surface *, int, int);
void drawCircle(int, int, int, int, int, int);
void drawCircleFromSurface(int, int, int);
void drawClippedImage(SDL_Surface *, int, int, int, int, int, int);
void drawBoxToMap(int, int, int, int, int, int, int);
SDL_Surface *addBorder(SDL_Surface *, int, int, int, int, int, int);
