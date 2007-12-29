#pragma once

#include <dialog.h>

namespace FwGui
{
	class Button;
	class ImageButton;
}

enum WifiTypeMenuSelection
{
	WTSEL_NONE,
	WTSEL_HOST_INTERNET,
	WTSEL_JOIN_INTERNET,
	WTSEL_HOST_LOCAL,
	WTSEL_JOIN_LOCAL,
	WTSEL_QUIT
};

class WifiTypeMenuDialog : public FwGui::Dialog
{
public:
	WifiTypeMenuDialog(bool enableInternet, bool enableLocal);
	virtual ~WifiTypeMenuDialog();

	virtual void ControlClicked(FwGui::Control* control);

	WifiTypeMenuSelection selection;

private:
	FwGui::Button* hostInternetButton;
	FwGui::Button* joinInternetButton;
	FwGui::Button* hostLocalButton;
	FwGui::Button* joinLocalButton;
	FwGui::ImageButton* closeButton;
};
