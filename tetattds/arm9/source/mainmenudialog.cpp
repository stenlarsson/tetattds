#include "tetattds.h"
#include "mainmenudialog.h"
#include <button.h>
#include <label.h>
#include <imagebutton.h>

using namespace FwGui;

MainMenuDialog::MainMenuDialog(bool enableReset)
:	Dialog("SELECT MODE", 6),
	selection(MMSEL_NONE)
{
	endlessButton = new Button(27, 46, 200, 24, "Endless");
	AddControl(endlessButton);
	vsSelfButton = new Button(27, 73, 200, 24, "Vs Self");
	AddControl(vsSelfButton);
	wifiButton = new Button(27, 100, 200, 24, "Wifi Play");
	AddControl(wifiButton);
	highscoresButton = new Button(27, 127, 200, 24, "Highscores");
	AddControl(highscoresButton);
	versionLabel = new Label(5, 158, 246, 32, VERSION_STRING "\n" ABOUT_STRING);
	AddControl(versionLabel);
	closeButton = new ImageButton(232, 0, STDIMG_CLOSE);
	AddControl(closeButton);
	if(!enableReset)
		closeButton->SetEnabled(false);
}

MainMenuDialog::~MainMenuDialog()
{
	delete endlessButton;
	delete vsSelfButton;
	delete wifiButton;
	delete highscoresButton;
	delete versionLabel;
	delete closeButton;
}

void MainMenuDialog::ControlClicked(Control* control)
{
	if(control == endlessButton)
	{
		selection = MMSEL_ENDLESS;
	}
	else if(control == vsSelfButton)
	{
		selection = MMSEL_VS_SELF;
	}
	else if(control == wifiButton)
	{
		selection = MMSEL_WIFI;
	}
	else if(control == highscoresButton)
	{
		selection = MMSEL_HIGHSCORES;
	}
	else if(control == closeButton)
	{
		selection = MMSEL_QUIT;
	}
}
