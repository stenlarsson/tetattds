#pragma once

enum AnimType
{
	ANIM_ONCE,
	ANIM_LOOPING,
	ANIM_CYCLING,
	ANIM_STATIC
};

class Anim
{
  public:
	Anim();
	~Anim();

	void Init(int numFrames, AnimType type);
	void AddFrame(int frame, int delay);

	void Tick();
	AnimType GetType() { return type; }
	int GetFrame() { return frames[curFrame]; }

	Anim* Copy();

  protected:
	int* frames;
	int* delays;
	int curFrame;
	int animTick;
	int numFrames;
	int step;
	enum AnimType type;
};
