#include "tetattds.h"
#include "anim.h"

Anim::Anim()
{
	frames = NULL;
	curFrame = 0;
	animTick = 0;
	delays = NULL;
}

Anim::~Anim()
{
	DEL(frames);
	DEL(delays);
}

void Anim::Init(int numFrames, AnimType type)
{
	DEL(frames);
	DEL(delays);
	this->type = type;
	frames = new int[numFrames];
	delays = new int[numFrames];
	for(int i = 0; i < numFrames; i++)
	{
		frames[i] = 0;
		delays[i] = 0;
	}
	curFrame = 0;
	this->numFrames = numFrames;
	step = 1;
}

void Anim::AddFrame(int frame, int delay)
{
	int i;
	// find empty frame
	for(i = 0;delays[i] != 0;i++);

	frames[i] = frame;
	delays[i] = delay;
}

void Anim::Tick()
{
	if(type == ANIM_STATIC)
		return;

	if(++animTick >= delays[curFrame])
	{
		animTick = 0;
		if(type == ANIM_CYCLING)
		{
			curFrame+=step;
			if(curFrame == numFrames-1)
			{
				step = -step;
			}
			else if(curFrame == 0)
			{
				step = -step;
			}
		}
		else if(type == ANIM_LOOPING)
		{
			curFrame += step;
			if(curFrame == numFrames)
				curFrame = 0;
		}
		else if(type == ANIM_ONCE)
		{
			curFrame += step;
			if(curFrame == numFrames)
			{
				type = ANIM_STATIC;
				curFrame--;
			}
		}
	}
}

Anim* Anim::Copy()
{
	Anim* newAnim = new Anim;
	newAnim->Init(numFrames, type);
	for(int i = 0;i < numFrames; i++)
	{
		newAnim->frames[i] = frames[i];
		newAnim->delays[i] = delays[i];
	}
	newAnim->curFrame = curFrame;
	newAnim->animTick = animTick;

	return newAnim;
}
