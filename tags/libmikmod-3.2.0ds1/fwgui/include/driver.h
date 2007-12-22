#pragma once

#include "inputlistener.h"

namespace FwGui
{
	class Dialog;
	
	class Driver
	{
	public:
		Driver();
		~Driver();
		
		void SetActiveDialog(Dialog* dialog);
		void SetListener(InputListener* listener);
		
		bool Tick();
	
	private:
		void InitGraphics();
	
		void* framebuffer;
		Dialog* dialog;
		int lastX;
		int lastY;
		bool firstTouch;
		bool needGraphicsInit;
		InputListener* listener;
	};
}
