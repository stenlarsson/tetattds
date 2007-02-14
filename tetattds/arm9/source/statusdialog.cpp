#include "tetattds.h"
#include "StatusDialog.h"
#include <imagebutton.h>
#include <label.h>

using namespace FwGui;

StatusDialog::StatusDialog(const char* title)
:	Dialog(title, 2),
	abort(false)
{
	closeButton = new ImageButton(232, 0, STDIMG_CLOSE);
	AddControl(closeButton, KEY_B);
	statusLabel = new Label(5, 79, 246, 32, "");
	AddControl(statusLabel);
}

StatusDialog::~StatusDialog()
{
	delete closeButton;
	delete statusLabel;
}

void StatusDialog::SetStatus(const char* status)
{
	statusLabel->SetText(status);
	Repaint();
}

void StatusDialog::ControlClicked(Control* control)
{
	if(control == closeButton)
	{
		abort = true;
	}
}
