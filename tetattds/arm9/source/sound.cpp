#include "tetattds.h"
#include "sound.h"
#include <stdlib.h>
#include "util.h"
#include "fifo.h"

#include "chain.h"

// sound data
#include "pop1_bin.h"
#include "pop2_bin.h"
#include "pop3_bin.h"
#include "pop4_bin.h"
#include "chain_bin.h"
#include "fanfare1_bin.h"
#include "fanfare2_bin.h"

// music
#include <mikmod9.h>

#include "tetattds_bin.h"

MODULE* Sound::song = NULL;
bool Sound::initialized = false;

void MikMod9_SendCommand(u32 command)
{
	SendFifo(command);
}

void Sound::InitMusic()
{
	if(initialized)
		return;

    /* register all the drivers */
    MikMod_RegisterDriver(&drv_nds_sw);

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
	
	initialized = true;
}

void Sound::LoadMusic()
{
	void* temp;
	int size;
	
	temp = Decompress(tetattds_bin, &size);
	song = Player_LoadMemory(temp, size, 64, 0);
	free(temp);
}

void Sound::PlayMusic(bool danger)
{
	Player_Start(song);
	Player_SetPosition(danger ? 0x2a : 0);
}

void Sound::StopMusic()
{
	Player_Stop();
}

void Sound::UnloadMusic()
{
	Player_Stop();
	Player_Free(song);
	song = NULL;
}

void Sound::UpdateMusic()
{
	MikMod_Update();
}

void Sound::PlayPopEffect(Chain* chain)
{
	TransferSoundData sound;
	
	switch(chain->length)
	{
	case 1:
		sound.data = pop1_bin;
		sound.len = pop1_bin_size;
		break;
		
	case 2:
		sound.data = pop2_bin;
		sound.len = pop2_bin_size;
		break;
		
	case 3:
		sound.data = pop3_bin;
		sound.len = pop3_bin_size;
		break;
		
	default:
		sound.data = pop4_bin;
		sound.len = pop4_bin_size;
		break;
	}
	
	sound.rate = 22050 + (chain->popCount * 1000);
	sound.vol = 127;
	sound.pan = 64;
	sound.format = 2;
	playSound(&sound);
	
	if(chain->popCount < 10)
	{
		chain->popCount++;
	}
}

void Sound::PlayDieEffect()
{
	Player_SetPosition(64);
}

void Sound::PlayChainStepEffect(Chain* chain)
{
	TransferSoundData sound;
	sound.data = chain_bin;
	sound.len = chain_bin_size;
	sound.rate = 22050;
	sound.vol = 127;
	sound.pan = 64;
	sound.format = 2;
	playSound(&sound);
}

void Sound::PlayChainEndEffect(Chain* chain)
{
	if(chain->length < 4)
	{
		return;
	}
	
	TransferSoundData sound;
	switch(chain->length)
	{
	case 4:
		sound.data = fanfare1_bin;
		sound.len = fanfare1_bin_size;
		break;
		
	default:
		sound.data = fanfare1_bin;
		sound.len = fanfare1_bin_size;
		break;
	}
	sound.rate = 22050;
	sound.vol = 127;
	sound.pan = 64;
	sound.format = 2;
	playSound(&sound);
}
