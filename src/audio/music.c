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

#include "../headers.h"

#include "music.h"
#include "../map.h"

extern Game game;

void loadMusic(char *name)
{
	char path[MAX_PATH_LENGTH];

	snprintf(path, sizeof(path), "%s", name);

	freeMusic();

	game.music = Mix_LoadMUS(path);

	if (game.music == NULL)
	{
		printf("Could not load music file %s: %s\n", name, Mix_GetError());
	}
}

void freeAllMusic()
{
	if (game.music != NULL)
	{
		stopMusic();

		Mix_FreeMusic(game.music);

		game.music = NULL;
	}
	
	if (game.bossMusic != NULL)
	{
		stopMusic();

		Mix_FreeMusic(game.bossMusic);

		game.bossMusic = NULL;
	}
}

void freeMusic()
{
	if (game.music != NULL)
	{
		stopMusic();

		Mix_FreeMusic(game.music);

		game.music = NULL;
	}
}

void playMusic()
{
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
	
	if (game.music == NULL)
	{
		loadMusic(getMapMusic());
	}
	
	Mix_PlayMusic(game.music, -1);
}

void stopMusic()
{
	Mix_HaltMusic();
}

void fadeOutMusic(int time)
{
	Mix_FadeOutMusic(time);
}

void fadeInMusic(int time)
{
	Mix_FadeInMusic(game.music, -1, time);
}

void setMusicVolume()
{
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

void pauseMusic()
{
	if (game.music != NULL)
	{
		if (Mix_PausedMusic())
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
	if (game.bossMusic == NULL)
	{
		loadBossMusic("music/terrortech_inc_.xm");
	}
	
	stopMusic();

	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
	
	Mix_PlayMusic(game.bossMusic, -1);
}

void loadBossMusic(char *name)
{
	char path[MAX_PATH_LENGTH];

	snprintf(path, sizeof(path), "%s", name);

	freeMusic();

	game.bossMusic = Mix_LoadMUS(path);

	if (game.music == NULL)
	{
		printf("Could not load music file %s: %s\n", name, Mix_GetError());
	}
}

void resumeMusic()
{
	playMusic();
}

void fadeBossMusic()
{
	fadeOutMusic(4000);
	
	Mix_HookMusicFinished(resumeMusic);
}
