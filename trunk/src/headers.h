/*
Copyright (C) 2009 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>
#include <zlib.h>
#include <libintl.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdarg.h>

#ifndef WIN32
	#include <sys/stat.h>
	#include <pwd.h>
	#include <unistd.h>
	#include <errno.h>
#else
	#include "strtok_r.h"
#endif

#include "SDL/SDL.h"
#include "SDL/SDL_endian.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"

#include "structs.h"
