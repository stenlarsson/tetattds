#pragma once

class EffectHandler;

class Effect
{
  public:
	virtual ~Effect() {}

	virtual void Draw() = 0;
	virtual void Tick() = 0;
	virtual bool IsDone() { return (duration <= 0); }

	virtual void SetOffset(int x, int y) = 0;

  protected:
	int duration;
	int XOffset;
	int YOffset;
};
