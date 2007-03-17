#pragma once

#include "control.h"
#include "graphics.h"
#include "inputlistener.h"

namespace FwGui
{
	class Dialog : public InputListener
	{
	public:
		Dialog(const char* title, int numcontrols);
		virtual ~Dialog();
	
		void Paint(Graphics* graphics);
		void Repaint() { repaint = true; }
		bool NeedsRepaint() { return repaint; }
		void SetRepaint(bool repaint) { this->repaint = repaint; }
		void Select(Control* control);
	
		virtual void KeyDown(Key key);
		virtual void TouchDown(int x, int y);
		virtual void ControlClicked(Control* control) = 0;
	
	protected:
		void AddControl(Control* control);
		void AddControl(Control* control, Key defaultKey);
	
		bool repaint;
		char* title;
		int numcontrols;
		int nextcontrol;
		Control** controls;
		int selectedControl;
		int defaultControl[FWGUI_NUM_KEYS];
	};
}
