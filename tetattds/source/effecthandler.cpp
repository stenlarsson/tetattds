#include "effecthandler.h"
#include "effect.h"
#include <algorithm>
#include <functional>

template <typename T>
static inline void delete_elem(T* & t)
{
	delete t;
	t = NULL;
}

template <typename It>
static inline void delete_each(It begin, It end)
{
	for (It it = begin; it != end; ++it)
		delete_elem(*it);
}

EffectHandler::EffectHandler()
  : effects()
{
}

EffectHandler::~EffectHandler()
{
	delete_each(effects.begin(), effects.end());
	effects.clear();
}

void EffectHandler::Add(Effect* e)
{
	effects.push_back(e);
}

void EffectHandler::Tick()
{
	std::for_each(effects.begin(), effects.end(), std::mem_fun(&Effect::Tick));
	std::list<Effect*>::iterator keep =
		std::partition(effects.begin(), effects.end(), std::mem_fun(&Effect::IsDone));
	delete_each(effects.begin(), keep);
	effects.erase(effects.begin(), keep);
}

void EffectHandler::Draw()
{
	std::for_each(effects.begin(), effects.end(), std::mem_fun(&Effect::Draw));
}
