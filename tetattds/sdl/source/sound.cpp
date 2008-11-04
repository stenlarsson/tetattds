#include "tetattds.h"
#include "sound.h"
#include <stdlib.h>

#include "chain.h"

#include <SDL/SDL_mixer.h>

Mix_Music *song = NULL;
Mix_Chunk *chain, *fanfare1, *fanfare2, *menu1, *menu2, *pop1, *pop2, *pop3, *pop4;
bool Sound::initialized = false;

void Sound::InitMusic()
{
	if(initialized)
		return;

	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,
					 MIX_DEFAULT_FORMAT,
					 2,
					 1024) == -1) {
		printf("Could not initialize sound, reason: %s\n", Mix_GetError());
		exit(2);
	}

	Mix_AllocateChannels(32);

	// TODO Mix_CloseAudio();

	initialized = true;
}

void Sound::LoadMusic()
{
	song = Mix_LoadMUS("music/tetattds.xm");
	if (!song){
		printf("Mix_LoadMUS(\"music/tetattds.xm\"): %s\n", Mix_GetError());
		exit(1);
	}

#define LOAD_SAMPLE(n) \
	n = Mix_LoadWAV((char*)"sound/" #n ".wav"); \
	if (!n) { \
		printf("Failed to load sample %s\n", "sound/" #n ".wav"); \
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
	if(Mix_PlayMusic(song, -1) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		return;
	}

	if(Mix_SetMusicPosition(danger ? 0x2a : 0) == -1) {
		printf("Mix_SetMusicPosition: %s\n", Mix_GetError());
		return;
	}
}

void Sound::StopMusic()
{
	Mix_HaltMusic();
}

void Sound::UnloadMusic()
{
	Mix_FreeMusic(song);
	song = NULL;
}

void Sound::UpdateMusic()
{
}

void pitch_effect(int chan, void *stream, int len, void *udata)
{
	float ratio = (22050 + (int)udata) / 22050.0f;
	short* samples = (short*)stream;
	int i = 0;
	for(float x = 0; x < len/2 - 1; x += ratio) {
		float p = x - int(x);
		samples[i++] = (1-p) * samples[int(x)] + p * samples[int(x) + 1];
	}

	for(; i < len/2; i++) {
		samples[i] = 0;
	}
}

void Sound::PlayPopEffect(Chain* chain)
{
	Mix_Chunk *sample;
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
	
	int channel = Mix_PlayChannel(-1, sample, 0);
	if(channel == -1) {
		printf("Mix_PlayChannel: %s\n", Mix_GetError());
		return;
	}

	if(!Mix_RegisterEffect(channel,
						   pitch_effect,
						   NULL,
						   (void*)(chain->popCount * 1000))) {
		printf("Mix_RegisterEffect: %s\n", Mix_GetError());
		return;
	}

	if(chain->popCount < 10)
	{
		chain->popCount++;
	}
}

void Sound::PlayDieEffect()
{
	if(Mix_PlayMusic(song, 0) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		return;
	}

	if(Mix_SetMusicPosition(64) == -1) {
		printf("Mix_SetMusicPosition: %s\n", Mix_GetError());
		return;
	}
}

void Sound::PlayChainStepEffect(Chain*)
{
	int channel = Mix_PlayChannel(-1, chain, 0);
	if(channel == -1) {
		printf("Mix_PlayChannel: %s\n", Mix_GetError());
		return;
	}
}

void Sound::PlayChainEndEffect(Chain* chain)
{
	if(chain->length < 4)
		return;
	
	Mix_Chunk *sample = (chain->length == 4) ? fanfare1 : fanfare2;
	int channel = Mix_PlayChannel(-1, sample, 0);
	if(channel == -1) {
		printf("Mix_PlayChannel: %s\n", Mix_GetError());
		return;
	}
}
