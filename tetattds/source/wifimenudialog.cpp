#include "tetattds.h"
#include "wifimenudialog.h"
#include <button.h>
#include <label.h>
#include <imagebutton.h>

using namespace FwGui;

WifiMenuDialog::WifiMenuDialog()
:	Dialog("WIFI PLAY", 5),
	selection(WMSEL_NONE),
	ready(false)
{
	readyButton = new Button(27, 70, 200, 24, "Set ready");
	AddControl(readyButton);
	selectLevelButton = new Button(27, 97, 200, 24, "Select level");
	AddControl(selectLevelButton);
	sendMessageButton = new Button(27, 124, 200, 24, "Send chat message");
	AddControl(sendMessageButton);
	closeButton = new ImageButton(232, 0, STDIMG_CLOSE);
	AddControl(closeButton, FWKEY_B);
	infoLabel = new Label(27, 43, 200, 24, "");
	AddControl(infoLabel);
}

WifiMenuDialog::~WifiMenuDialog()
{
	delete infoLabel;
	delete readyButton;
	delete selectLevelButton;
	delete sendMessageButton;
	delete closeButton;
}

void WifiMenuDialog::SetInfo(const char* info)
{
	infoLabel->SetText(info);
}

void WifiMenuDialog::ControlClicked(Control* control)
{
	if(control == readyButton)
	{
		if(ready)
		{
			readyButton->SetText("Set ready");
			selectLevelButton->SetEnabled(true);
			sendMessageButton->SetEnabled(true);
			ready = false;
			selection = WMSEL_NOT_READY;
		}
		else
		{
			readyButton->SetText("Set not ready");
			selectLevelButton->SetEnabled(false);
			sendMessageButton->SetEnabled(false);
			ready = true;
			selection = WMSEL_READY;
		}
		Repaint();
	}
	else if(control == selectLevelButton)
	{
		selection = WMSEL_LEVEL;
	}
	else if(control == sendMessageButton)
	{
		selection = WMSEL_MESSAGE;
	}
	else if(control == closeButton)
	{
		selection = WMSEL_QUIT;
	}
}
