#pragma once

#include <dialog.h>

namespace FwGui
{
	class Label;
	class Button;
	class ImageButton;
}

enum WifiMenuSelection
{
	WMSEL_NONE,
	WMSEL_READY,
	WMSEL_NOT_READY,
	WMSEL_LEVEL,
	WMSEL_MESSAGE,
	WMSEL_QUIT
};

class WifiMenuDialog : public FwGui::Dialog
{
public:
	WifiMenuDialog();
	virtual ~WifiMenuDialog();

	void SetInfo(const char* info);

	virtual void ControlClicked(FwGui::Control* control);

	WifiMenuSelection selection;

private:
	bool ready;
	FwGui::Label* infoLabel;
	FwGui::Button* readyButton;
	FwGui::Button* selectLevelButton;
	FwGui::Button* sendMessageButton;
	FwGui::ImageButton* closeButton;
};
