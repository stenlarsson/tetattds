#include "tetattds.h"
#include "effready.h"
#include "fieldgraphics.h"

EffReady::EffReady()
{
	duration = 150;
}

EffReady::~EffReady()
{
}

void EffReady::Draw()
{
	if(duration > 100)
		g_fieldGraphics->PrintCountdown(3);
	else if(duration > 50)
		g_fieldGraphics->PrintCountdown(2);
	else if(duration > 1)
		g_fieldGraphics->PrintCountdown(1);
	else if(duration <= 1)
		g_fieldGraphics->PrintCountdown(0);
}

void EffReady::Tick()
{
	duration--;
}
