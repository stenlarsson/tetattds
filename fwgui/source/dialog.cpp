#include "fwgui.h"
#include "dialog.h"
#include "theme.h"
#include <string.h>
#include <limits.h>

namespace FwGui
{
	Dialog::Dialog(const char* title, int numcontrols)
	:	repaint(true),
		title(strdup(title)),
		numcontrols(numcontrols),
		nextcontrol(0),
		controls(new Control*[numcontrols]),
		selectedControl(0)
	{
		for(int i = 0;i<14;i++)
			defaultControl[i] = -1;
	}
	
	Dialog::~Dialog()
	{
		free(title);
		delete controls;
	}
	
	void Dialog::Paint(Graphics* graphics)
	{
		graphics->SetFillColor(titleBackgroundColor);
		graphics->FillRect(0, 0, 256, 24);
		graphics->SetFont(Font::GUNSHIP_12);
		graphics->SetPenColor(titleTextColor);
		graphics->DrawString(6, 17, title);
	
		for(int i = 0; i < numcontrols; i++)
		{
			controls[i]->Paint(graphics);
		}
	
		controls[selectedControl]->PaintOutline(graphics);
	}
	
	void Dialog::AddControl(Control* control)
	{
		FWGUI_ASSERT(nextcontrol < numcontrols);
	
		controls[nextcontrol] = control;
		nextcontrol++;
	}
	
	void Dialog::AddControl(Control* control, Key defaultKey)
	{
		FWGUI_ASSERT(defaultControl[defaultKey] == -1);
		defaultControl[defaultKey] = nextcontrol;
		
		AddControl(control);
	}
	
	void Dialog::Select(Control* control)
	{
		for(int i = 0; i < numcontrols; i++)
		{
			if(controls[i] == control)
			{
				selectedControl = i;
			}
		}
		Repaint();
	}

	void Dialog::KeyDown(Key key)
	{
		int control = defaultControl[key];

		if(control != -1) {
			if(controls[control]->IsEnabled()) {
				ControlClicked(controls[control]);
			}
			return;
		}

		switch(key) {
		case FWKEY_A:
			if(controls[selectedControl]->IsEnabled()) {
				ControlClicked(controls[control]);
			}
			break;

		case FWKEY_RIGHT:
		case FWKEY_LEFT:
		case FWKEY_UP:
		case FWKEY_DOWN:
			int startX = controls[selectedControl]->x;
			int startY = controls[selectedControl]->y;
			
			int distance = INT_MAX;
			for(int i = 0; i < numcontrols; i++)
			{
				int dx = INT_MAX;
				int dy = INT_MAX;
				switch(key) {
				case FWKEY_RIGHT:
					if(controls[i]->x > startX) {
						int dx = controls[i]->x - startX;
						int dy = controls[i]->y - startY;
					}
					break;
				case FWKEY_LEFT:
					if(controls[i]->x + controls[i]->width < startX) {
						int dx = controls[i]->x + controls[i]->width - startX;
						int dy = controls[i]->y - startY;
					}
					break;
				case FWKEY_UP:
					if(controls[i]->y + controls[i]->height < startY) {
						int dx = controls[i]->x - startX;
						int dy = controls[i]->y + controls[i]->height - startY;
					}
					break;
				case FWKEY_DOWN:
					if(controls[i]->y > startY) {
						int dx = controls[i]->x - startX;
						int dy = controls[i]->y - startY;
					}
					break;
				default:
					break;
				}

				if(dx*dx + dy*dy < distance)
				{
					if(controls[i]->IsSelectable())
					{
						selectedControl = i;
						distance = dx*dx + dy*dy;
					}
				}
			}
			Repaint();
			break;
		}
	}

	void Dialog::TouchDown(int x, int y)
	{
		for(int i = 0; i < numcontrols; i++)
		{
			if(controls[i]->Hit(x, y))
			{
				if(controls[i]->IsSelectable())
				{
					selectedControl = i;
				}
				if(controls[i]->IsEnabled())
				{
					ControlClicked(controls[i]);
				}
				Repaint();
			}
		}
	}
}
