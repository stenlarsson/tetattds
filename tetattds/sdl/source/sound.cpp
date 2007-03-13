#include "tetattds.h"
#include "sound.h"
#include <stdlib.h>

MODULE* Sound::song = NULL;
bool Sound::initialized = false;

void Sound::InitMusic()
{
	if(initialized)
		return;

#if 0
	/* register all the drivers */
	MikMod_RegisterDriver(&drv_sw);

	/* register all the module loaders */
	MikMod_RegisterLoader(&load_xm);

	/* initialize the library */
	md_mode |= DMODE_SOFT_MUSIC;
	md_mixfreq = 22050; 
	if (MikMod_Init(""))
	{
		fprintf(stderr, "Could not initialize sound, reason: %s\n",
		        MikMod_strerror(MikMod_errno));
		return;
	}
#endif
	
	initialized = true;
}

void Sound::LoadMusic()
{
	// TODO: Implement
}

void Sound::PlayMusic(bool danger)
{
#if 0
	Player_Start(song);
	Player_SetPosition(danger ? 0x2a : 0);
#endif
}

void Sound::StopMusic()
{
	// Player_Stop();
}

void Sound::UnloadMusic()
{
#if 0
	Player_Stop();
	Player_Free(song);
	song = NULL;
#endif
}

void Sound::UpdateMusic()
{
	// MikMod_Update();
}

void Sound::PlayPopEffect(Chain* chain)
{
	// TODO: Implement
}

void Sound::PlayDieEffect()
{
	// Player_SetPosition(64);
}

void Sound::PlayChainStepEffect(Chain* chain)
{
	// TODO: Implement
}

void Sound::PlayChainEndEffect(Chain* chain)
{
	// TODO: Implement
}
