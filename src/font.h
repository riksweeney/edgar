TTF_Font *loadFont(char *, int);
void closeFont(TTF_Font *);
void drawString(char *, int, int, TTF_Font *, int, int);
SDL_Surface *generateTextSurface(char *, TTF_Font *);
