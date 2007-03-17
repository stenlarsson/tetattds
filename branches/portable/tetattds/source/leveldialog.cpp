#include "tetattds.h"
#include "leveldialog.h"
#include <button.h>
#include <imagebutton.h>

using namespace FwGui;

LevelDialog::LevelDialog(int defaultLevel)
:	Dialog("SELECT LEVEL", NUM_LEVELS+1),
	level(-1)
{
	int x = 60;
	int y = 80;
	
	for(int i = 0; i < NUM_LEVELS; i++)
	{
		char str[3];
		sprintf(str, "%i", i+1);
		buttons[i] = new Button(x, y, 24, 24, str);
		AddControl(buttons[i]);
		x += 28;
		if(i == 4)
		{
			x = 60;
			y += 28;
		}
	}
	
	closeButton = new ImageButton(232, 0, STDIMG_CLOSE);
	AddControl(closeButton, FWKEY_B);

	if(defaultLevel >= 0 && defaultLevel < 10)
	{
		Select(buttons[defaultLevel]);
	}
}

LevelDialog::~LevelDialog()
{
	for(int i = 0; i < NUM_LEVELS; i++)
	{
		delete buttons[i];
	}
	delete closeButton;
}

void LevelDialog::ControlClicked(Control* control)
{
	for(int i = 0; i < NUM_LEVELS; i++)
	{
		if(control == buttons[i])
		{
			level = i;
		}
	}
	
	if(control == closeButton)
		level = -2;
}
