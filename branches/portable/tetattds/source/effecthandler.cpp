#include "util.h"
#include "effecthandler.h"
#include "effect.h"
#include <algorithm>
#include <functional>

EffectHandler::EffectHandler()
  : effects()
{
}

EffectHandler::~EffectHandler()
{
	delete_and_clear(effects);
}

void EffectHandler::Add(Effect* e)
{
	effects.push_back(e);
}

void EffectHandler::Tick()
{
	std::for_each(effects.begin(), effects.end(), std::mem_fun(&Effect::Tick));
	delete_and_erase_if(effects, std::mem_fun(&Effect::IsDone));
}

void EffectHandler::Draw()
{
	std::for_each(effects.begin(), effects.end(), std::mem_fun(&Effect::Draw));
}
