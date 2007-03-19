#pragma once

class EffectHandler;

class Effect
{
public:
	Effect(int x, int y, int duration)
		: XOffset(x), YOffset(y), duration(duration) {}
	virtual ~Effect() {}

	virtual void Draw() = 0;
	virtual void Tick() { duration--; }
	bool IsDone() const { return (duration <= 0); }

protected:
	int XOffset;
	int YOffset;
	int duration;
};
