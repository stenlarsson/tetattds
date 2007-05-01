#pragma once

#include "fieldgraphics.h"

class EffectHandler;

class Effect
{
public:
	Effect(int pos, int duration)
		: XOffset(g_fieldGraphics->GetFieldX(pos)),
		  YOffset(g_fieldGraphics->GetFieldY(pos, true)),
		  duration(duration)
	{}
	virtual ~Effect() {}

	virtual void Draw() = 0;
	virtual void Tick() { duration--; }
	bool IsDone() const { return (duration <= 0); }

protected:
	int XOffset;
	int YOffset;
	int duration;
};
