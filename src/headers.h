#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <png.h>
#include <ctype.h>
#include <locale.h>
#include <zlib.h>
#include <libintl.h>

#ifndef WIN32
	#include <sys/stat.h>
	#include <pwd.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"

#include "structs.h"
