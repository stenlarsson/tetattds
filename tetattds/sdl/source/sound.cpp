#include "tetattds.h"
#include "sound.h"
#include <stdlib.h>

#include "chain.h"

MODULE* Sound::song = NULL;
SAMPLE *chain, *fanfare1, *fanfare2, *menu1, *menu2, *pop1, *pop2, *pop3, *pop4;
bool Sound::initialized = false;

void Sound::InitMusic()
{
	if(initialized)
		return;

	/* register all the drivers */
	MikMod_RegisterAllDrivers();

	/* register fasttracker module loader */
	MikMod_RegisterLoader(&load_xm);

	/* initialize the library */
	if (MikMod_Init(""))
	{
		fprintf(stderr, "Could not initialize sound, reason: %s\n",
		        MikMod_strerror(MikMod_errno));
		return;
	}
	
  /* reserve 2 voices for sound effects */
  MikMod_SetNumVoices(-1, 2);

	initialized = true;
}

void Sound::LoadMusic()
{
	song = Player_Load("music/tetattds.xm", 64, 0);
	if (!song){
		fprintf(stderr,"Failed to load music\n");
		exit(1);
	}

#define LOAD_SAMPLE(n) \
  n = Sample_Load("sound/" #n ".wav"); \
	if (!n) { \
		fprintf(stderr, "Failed to load sample %s\n", "sound/" #n ".wav"); \
		exit(1); \
	} 
	
	LOAD_SAMPLE(chain);
	LOAD_SAMPLE(fanfare1);
	LOAD_SAMPLE(fanfare2);
	LOAD_SAMPLE(menu1);
	LOAD_SAMPLE(menu2);
	LOAD_SAMPLE(pop1);
	LOAD_SAMPLE(pop2);
	LOAD_SAMPLE(pop3);
	LOAD_SAMPLE(pop4);

#undef LOAD_SAMPLE	
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
	SAMPLE *sample;
	switch(chain->length)
	{
	case 1:
		sample = pop1;
		break;
		
	case 2:
		sample = pop2;
		break;

	case 3:
		sample = pop3;
		break;

	default:
		sample = pop4;
		break;
	}
	
	int voice = Sample_Play(sample, 0, 0);
	Voice_SetFrequency(voice, sample->speed + (chain->popCount * 1000));

	if(chain->popCount < 10)
	{
		chain->popCount++;
	}
}

void Sound::PlayDieEffect()
{
	Player_SetPosition(64);
}

void Sound::PlayChainStepEffect(Chain*)
{
	Sample_Play(chain, 0, 0);
}

void Sound::PlayChainEndEffect(Chain* chain)
{
	if(chain->length < 4)
		return;
	
	Sample_Play((chain->length == 4) ? fanfare1 : fanfare2, 0, 0);
}
