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

static Mix_Music *music;

void freeMusic(void);

void loadMusic(char *name)
{
	char path[MAX_PATH_LENGTH];

	snprintf(path, sizeof(path), "%s", name);

	freeMusic();

	music = Mix_LoadMUS(path);

	if (music == NULL)
	{
		printf("Could not load music file %s: %s\n", name, Mix_GetError());
	}
}

void freeMusic()
{
	if (music != NULL)
	{
		stopMusic();

		Mix_FreeMusic(music);

		music = NULL;
	}
}

void playMusic()
{
	if (music != NULL)
	{
		Mix_VolumeMusic(MIX_MAX_VOLUME);

		Mix_PlayMusic(music, -1);
	}
}

void stopMusic()
{
	Mix_HaltMusic();
}

int adjustMusicVolume(int val)
{
	int current = Mix_VolumeMusic(-1);

	if ((current == 0 && val < 0) || (current == MIX_MAX_VOLUME && val > 0))
	{
		return current;
	}

	current += val;

	Mix_VolumeMusic(current);

	return current;
}

void setMusicVolume(int val)
{
	Mix_VolumeMusic(val);
}

int getMusicVolume()
{
	return Mix_VolumeMusic(-1);
}

void pauseMusic()
{
	if (music != NULL)
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

void loadBossMusic()
{
	loadMusic("music/battle_of_the_fireflies.s3m");

	playMusic();
}
