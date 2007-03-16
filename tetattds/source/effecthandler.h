#pragma once

#include <list>
class Effect;

/**
 * Keeper of Effect objects.
 * 
 * Effects should be allocated with new, and then added to the
 * handler using Add. The handler assumes ownership and will
 * delete the Effects as soon as the effect is completed.
 *
 * The Tick and Draw methods call the corresponding methods
 * once for every stored effect object.
 */
class EffectHandler
{
  public:
	EffectHandler();
	~EffectHandler();

	void Add(Effect* newEff);
	void Tick();
	void Draw();

  private:
	std::list<Effect*> effects;
};
