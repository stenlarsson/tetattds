#include "tetattds.h"
#include "wifitypemenudialog.h"
#include <button.h>
#include <imagebutton.h>

using namespace FwGui;

WifiTypeMenuDialog::WifiTypeMenuDialog(bool enableInternet, bool enableLocal)
:	Dialog("WIFI TYPE", 5),
	selection(WTSEL_NONE)
{
	hostInternetButton = new Button(27, 46, 200, 24, "Host Internet");
	AddControl(hostInternetButton);
	joinInternetButton = new Button(27, 73, 200, 24, "Join Internet");
	AddControl(joinInternetButton);
	hostLocalButton = new Button(27, 100, 200, 24, "Host Local");
	AddControl(hostLocalButton);
	joinLocalButton = new Button(27, 127, 200, 24, "Join Local");
	AddControl(joinLocalButton);
	closeButton = new ImageButton(232, 0, STDIMG_CLOSE);
	AddControl(closeButton, FWKEY_B);
	
	hostInternetButton->SetEnabled(enableInternet);
	joinInternetButton->SetEnabled(enableInternet);
	hostLocalButton->SetEnabled(enableLocal);
	joinLocalButton->SetEnabled(enableLocal);
}

WifiTypeMenuDialog::~WifiTypeMenuDialog()
{
	delete hostInternetButton;
	delete joinInternetButton;
	delete hostLocalButton;
	delete joinLocalButton;
	delete closeButton;
}

void WifiTypeMenuDialog::ControlClicked(Control* control)
{
	if(control == hostInternetButton)
	{
		selection = WTSEL_HOST_INTERNET;
	}
	else if(control == joinInternetButton)
	{
		selection = WTSEL_JOIN_INTERNET;
	}
	else if(control == hostLocalButton)
	{
		selection = WTSEL_HOST_LOCAL;
	}
	else if(control == joinLocalButton)
	{
		selection = WTSEL_JOIN_LOCAL;
	}
	else if(control == closeButton)
	{
		selection = WTSEL_QUIT;
	}
}
