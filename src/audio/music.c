/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "../headers.h"

#include "music.h"
#include "../map.h"
#include "../system/pak.h"

extern Game game;

static void playGameOverMusic(void);

void loadMusic(char *name)
{
	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	freeMusic();

	game.music = loadMusicFromPak(name);

	if (game.music == NULL)
	{
		printf("Could not load music file %s: %s\n", name, Mix_GetError());
	}
}

void freeMusic()
{
	stopMusic();
	
	if (game.music != NULL)
	{
		Mix_FreeMusic(game.music);

		game.music = NULL;
	}
}

void playMusic()
{
	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}
	
	freeMusic();

	if (game.music == NULL)
	{
		loadMusic(getMapMusic());
	}
	
	Mix_VolumeMusic(MIX_MAX_VOLUME);

	Mix_PlayMusic(game.music, -1);
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
	Mix_FadeInMusic(game.music, -1, time);
}

void setMusicVolume()
{
	Mix_VolumeMusic(MIX_MAX_VOLUME);
}

void pauseMusic(int pause)
{
	if (game.music != NULL)
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

void playBossMusic()
{
	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	freeMusic();

	loadBossMusic("music/terrortech_inc_.xm");

	Mix_VolumeMusic(MIX_MAX_VOLUME);

	Mix_PlayMusic(game.music, -1);
}

void loadBossMusic(char *name)
{
	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	freeMusic();

	game.music = loadMusicFromPak(name);

	if (game.music == NULL)
	{
		printf("Could not load music file %s: %s\n", name, Mix_GetError());
	}
}

void resumeMusic()
{
	Mix_HookMusicFinished(NULL);

	playMusic();
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
	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	Mix_HookMusicFinished(NULL);

	loadMusic("music/oxide_-_sadness.xm");

	playMusic();
}
