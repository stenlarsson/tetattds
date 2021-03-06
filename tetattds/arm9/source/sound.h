#pragma once

class Chain;

#include <mikmod.h>

class Sound
{
public:
	static void PlayPopEffect(Chain* chain);
	static void PlayDieEffect();
	static void PlayChainStepEffect(Chain* chain);
	static void PlayChainEndEffect(Chain* chain);
	
	static void InitMusic();
	static void LoadMusic();
	static void UnloadMusic();
	static void PlayMusic(bool danger);
	static void StopMusic();
	static void UpdateMusic();
	
	static MODULE* song;
private:
	
	static bool initialized;
};
