/*
Copyright (C) 2009-2013 Parallel Realities

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
static char *getSongName(char *, int);

static char *oldMusicTracks[] = {
			"TITLE_MUSIC", "music/march13.it",
			"MAP01_MUSIC", "music/jk_village.xm",
			"MAP02_MUSIC", "music/cavesii.xm",
			"MAP03_MUSIC", "music/countryside.mod",
			"MAP04_MUSIC", "music/forbidden_zone.mod",
			"MAP05_MUSIC", "music/caverns_of_time.xm",
			"MAP06_MUSIC", "music/town4.xm",
			"MAP07_MUSIC", "music/exploring_new_worlds.xm",
			"MAP08_MUSIC", "music/aow-macabre.xm",
			"MAP09_MUSIC", "music/battle_of_the_fireflies.s3m",
			"MAP10_MUSIC", "music/technology.xm",
			"MAP11_MUSIC", "music/bookofshadows.xm",
			"MAP12_MUSIC", "music/cavesii.xm",
			"MAP13_MUSIC", "music/mystery.it",
			"MAP14_MUSIC", "NO_MUSIC",
			"MAP15_MUSIC", "music/cavern.it",
			"MAP16_MUSIC", "music/climax.it",
			"MAP17_MUSIC", "music/trapped.xm",
			"MAP18_MUSIC", "music/fb-snow.it",
			"MAP19_MUSIC", "music/book_of_desire_2.xm",
			"MAP20_MUSIC", "music/bj_unkow.s3m",
			"MAP21_MUSIC", "music/horror.it",
			"MAP22_MUSIC", "music/mystery.xm",
			"MAP23_MUSIC", "music/battle_cry.mod",
			"MAP24_MUSIC", "music/climax.it",
			"MAP25_MUSIC", "music/crypt.xm",
			"MAP26_MUSIC", "music/szc2_-_fight_for_your_lives.xm",
			"MAP25_MUSIC", "music/crypt",
			"TUTORIAL_MUSIC", "music/jk_village.xm",
			"CREDITS_MUSIC", "music/credits.mod",
			"GAMEOVER_MUSIC", "music/oxide_-_sadness.xm",
			"BOSS_MUSIC", "music/terrortech_inc_.xm",
			"EVIL_EDGAR_MUSIC", "music/battle_for_life.xm",
			"SORCEROR_MUSIC", "music/dgt_enemy.it",
			"REUNION_MUSIC", "music/GSLINGER.MOD"
};

static char *newMusicTracks[] = {
			"TITLE_MUSIC", "music/Menu_loop.mp3",
			"MAP01_MUSIC", "music/Lazy Day v0_9.mp3",
			"MAP02_MUSIC", "music/Ruinous Laments(5.6).mp3",
			"MAP03_MUSIC", "music/Devilish design.mp3",
			"MAP04_MUSIC", "music/Avgvst - Mushrooms_1.ogg",
			"MAP05_MUSIC", "music/MysticalCaverns.ogg",
			"MAP06_MUSIC", "music/Red Curtain.mp3",
			"MAP07_MUSIC", "music/ratsrats_0.ogg",
			"MAP08_MUSIC", "music/beauty_of_chaos.ogg",
			"MAP09_MUSIC", "music/Battle in the winter.mp3",
			"MAP10_MUSIC", "music/Devilish design.mp3",
			"MAP11_MUSIC", "music/Decision.mp3",
			"MAP12_MUSIC", "music/Ruinous Laments(5.6).mp3",
			"MAP13_MUSIC", "music/Heroic Minority.mp3",
			"MAP14_MUSIC", "NO_MUSIC",
			"MAP15_MUSIC", "music/Mystical Theme.mp3",
			"MAP16_MUSIC", "music/Battle.mp3",
			"MAP17_MUSIC", "music/Szymon Matuszewski - Hope.mp3",
			"MAP18_MUSIC", "music/A wintertale_0.mp3",
			"MAP19_MUSIC", "music/MysticalCaverns.ogg",
			"MAP20_MUSIC", "music/Steeps of Destiny.mp3",
			"MAP21_MUSIC", "music/Medicine.mp3",
			"MAP22_MUSIC", "music/Decision.mp3",
			"MAP23_MUSIC", "music/Heroic Minority.mp3",
			"MAP24_MUSIC", "music/Battle.mp3",
			"MAP25_MUSIC", "music/Medicine.mp3",
			"MAP26_MUSIC", "music/Undead Rising.mp3",
			"TUTORIAL_MUSIC", "music/Lazy Day v0_9.mp3",
			"CREDITS_MUSIC", "music/Heroes Theme_0.mp3",
			"GAMEOVER_MUSIC", "music/Without.mp3",
			"BOSS_MUSIC", "music/So, let see, what you can_0.mp3",
			"EVIL_EDGAR_MUSIC", "music/So, let see, what you can_0.mp3",
			"SORCEROR_MUSIC", "music/Zander Noriega - Fight Them Until We Cant.mp3",
			"REUNION_MUSIC", "music/Lazy Day v0_9.mp3"
};
static int musicTracksLength = sizeof(newMusicTracks) / sizeof(char *);

void loadMusic(char *musicToLoad)
{
	char *name;
	
	if (strcmpignorecase(musicName, musicToLoad) != 0)
	{
		STRNCPY(musicName, musicToLoad, MAX_VALUE_LENGTH);
	}

	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	freeMusic();
	
	name = getSongName(musicToLoad, 0);

	if (name == NULL || strcmpignorecase(name, "NO_MUSIC") == 0)
	{
		return;
	}

	music = loadMusicFromPak(name);
	
	if (music != NULL)
	{
		return;
	}
	
	name = getSongName(musicToLoad, 1);

	if (name == NULL || strcmpignorecase(name, "NO_MUSIC") == 0)
	{
		return;
	}

	music = loadMusicFromPak(name);
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
	loadMusic("BOSS_MUSIC");

	#if DEV == 1
	if (game.gameType == REPLAYING)
	{
		printf("%f BOSS_MUSIC\n", (float)game.frames / 60);
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
	STRNCPY(musicName, "GAMEOVER_MUSIC", MAX_VALUE_LENGTH);

	if (game.audio == FALSE || game.musicDefaultVolume == 0)
	{
		return;
	}

	Mix_HookMusicFinished(NULL);

	loadMusic("GAMEOVER_MUSIC");

	Mix_PlayMusic(music, -1);
	
	Mix_VolumeMusic(game.musicDefaultVolume * VOLUME_STEPS);
}

static char *getSongName(char *name, int oldSong)
{
	int i;
	
	if (oldSong == 1)
	{
		for (i=0;i<musicTracksLength;i+=2)
		{
			if (strcmpignorecase(name, oldMusicTracks[i]) == 0)
			{
				return oldMusicTracks[i + 1];
			}
		}
	}
	
	else
	{
		for (i=0;i<musicTracksLength;i+=2)
		{
			if (strcmpignorecase(name, newMusicTracks[i]) == 0)
			{
				return newMusicTracks[i + 1];
			}
		}
	}
	
	printf("Could not match song: %s\n", name);
	
	return NULL;
}
