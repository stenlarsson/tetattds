#pragma once

#include <memory>

enum AnimType
{
	ANIM_ONCE,
	ANIM_LOOPING,
	ANIM_CYCLING,
	ANIM_STATIC
};

struct AnimFrame
{
	AnimFrame() {}
	AnimFrame(int tile, int delay = 1)
		: tile(tile), delay(delay) {}

	int tile;
	int delay;
};

class BaseAnim
{
public:
	BaseAnim(AnimType type, AnimFrame const * begin, AnimFrame const * end);
	~BaseAnim();

protected:
	AnimFrame* frames;
	AnimFrame* curFrame;
	AnimFrame* endFrame;
	int animTick;
	int step;
	AnimType type;
};

struct Anim : public BaseAnim
{
	Anim(AnimFrame const & frame)
		: BaseAnim(ANIM_STATIC, &frame, &frame+1)
	{}

	Anim(
		AnimType type,
		AnimFrame const * begin,
		size_t count)
		: BaseAnim(type, begin, begin+count)
	{}

	Anim(Anim const & old)
		: BaseAnim(old.type, old.frames, old.endFrame)
	{}

	Anim & operator=(Anim const & old)
	{
		ASSERT(this != &old);

		this->~Anim();
		new (this) Anim(old);

		return *this;
	}

	void Tick();
	bool IsDone() { return type == ANIM_STATIC; }
	int GetFrame() { return curFrame->tile; }
};
