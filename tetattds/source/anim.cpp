#include "tetattds.h"
#include "anim.h"
#include <algorithm>

BaseAnim::BaseAnim(
	AnimType type, AnimFrame const * begin, AnimFrame const * end)
	: frames((AnimFrame*)new char[sizeof(AnimFrame)*(end-begin)]),
		curFrame(frames),
		endFrame(frames + (end-begin)),
		animTick(0),
		step(1),
		type(type)
{
	std::uninitialized_copy(begin, end, frames);
}

BaseAnim::~BaseAnim()
{
	delete[] frames;
	frames = curFrame = endFrame = NULL;
}

void Anim::Tick()
{	
	if(type == ANIM_STATIC)
		return;

	if(++animTick >= curFrame->delay)
	{
		animTick = 0;
		curFrame++; // curFrame += step;
		if(type == ANIM_CYCLING)
		{
			if(curFrame == 0 || curFrame == endFrame-1)
				step = -step;
		}
		else if(type == ANIM_LOOPING)
		{
			if(curFrame == endFrame)
				curFrame = frames;
		}
		else if(type == ANIM_ONCE)
		{
			if(curFrame == endFrame)
			{
				type = ANIM_STATIC;
				curFrame--;
			}
		}
	}
}
