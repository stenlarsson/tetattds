#pragma once

class Effect;

struct EffList
{
  public:
	Effect* effect;
	EffList* next;
};

class EffectHandler
{
  public:
	EffectHandler();
	~EffectHandler();

	void Add(Effect* newEff);
	void Tick();
	void Draw();

  private:
	Effect* GetEffects();
	void Remove(Effect* remEff);

	EffList* first;
	EffList* last;
	EffList* current;
	bool bGet;
};
