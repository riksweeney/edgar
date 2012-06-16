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

#include "../headers.h"

#include "../map.h"
#include "../system/pak.h"
#include "music.h"

extern Game game;

static void playGameOverMusic(void);

static Mix_Music *music;
static char musicName[MAX_VALUE_LENGTH];

void loadMusic(char *name)
{
	if (strcmpignorecase(musicName, name) != 0)
	{
		STRNCPY(musicName, name, MAX_VALUE_LENGTH);
	}

	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	freeMusic();

	if (strcmpignorecase(name, "NO_MUSIC") == 0)
	{
		return;
	}

	music = loadMusicFromPak(name);

	if (music == NULL)
	{
		printf("Could not load music file %s: %s\n", name, Mix_GetError());
	}
}

void freeMusic()
{
	stopMusic();

	if (music != NULL)
	{
		Mix_FreeMusic(music);

		music = NULL;
	}
}

void playLoadedMusic()
{
	if (strcmpignorecase(musicName, "NO_MUSIC") == 0)
	{
		return;
	}

	loadMusic(musicName);

	Mix_PlayMusic(music, -1);
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

void playMapMusic()
{
	loadMusic(getMapMusic());

	Mix_PlayMusic(music, -1);
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

void stopMusic()
{
	Mix_HaltMusic();
}

void fadeOutMusic(int time)
{
	Mix_HookMusicFinished(NULL);

	Mix_FadeOutMusic(time);
}

void fadeInMusic(int time)
{
	Mix_FadeInMusic(music, -1, time);
}

void pauseMusic(int pause)
{
	if (music != NULL)
	{
		if (pause == FALSE)
		{
			Mix_ResumeMusic();
		}

		else
		{
			Mix_PauseMusic();
		}
	}
}

void updateMusicVolume()
{
	Mix_ResumeMusic();
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

void playDefaultBossMusic()
{
	loadMusic("music/terrortech_inc_.xm");

	#if DEV == 1
	if (game.gameType == REPLAYING)
	{
		printf("%f music/terrortech_inc_.xm\n", (float)game.frames / 60);
	}
	#endif

	Mix_PlayMusic(music, -1);
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

void playBossMusic(char *name)
{
	loadMusic(name);

	#if DEV == 1
	if (game.gameType == REPLAYING)
	{
		printf("%f %s\n", (float)game.frames / 60, name);
	}
	#endif

	Mix_PlayMusic(music, -1);
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

void resumeMusic()
{
	Mix_HookMusicFinished(NULL);

	playMapMusic();
}

void fadeBossMusic()
{
	fadeOutMusic(4000);

	Mix_HookMusicFinished(resumeMusic);
}

void loadGameOverMusic()
{
	fadeOutMusic(2000);

	Mix_HookMusicFinished(playGameOverMusic);
}

static void playGameOverMusic()
{
	STRNCPY(musicName, "music/oxide_-_sadness.xm", MAX_VALUE_LENGTH);

	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	Mix_HookMusicFinished(NULL);

	loadMusic("music/oxide_-_sadness.xm");

	Mix_PlayMusic(music, -1);
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}
