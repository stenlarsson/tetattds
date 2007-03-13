
#pragma once

class State {
public:
	virtual ~State() {}
	virtual void Enter() = 0;
	virtual void Tick() = 0;
	virtual void Exit() = 0;
};

void InitStates();
void StateTick();

