#pragma once

#include <dialog.h>

namespace FwGui
{
	class Button;
	class Label;
	class ImageButton;
}

enum MainMenuSelection
{
	MMSEL_NONE,
	MMSEL_ENDLESS,
	MMSEL_VS_SELF,
	MMSEL_WIFI,
	MMSEL_HIGHSCORES,
	MMSEL_QUIT
};

class MainMenuDialog : public FwGui::Dialog
{
public:
	MainMenuDialog(bool enableReset);
	virtual ~MainMenuDialog();

	virtual void ControlClicked(FwGui::Control* control);

	MainMenuSelection selection;

private:
	FwGui::Button* endlessButton;
	FwGui::Button* vsSelfButton;
	FwGui::Button* wifiButton;
	FwGui::Button* highscoresButton;
	FwGui::Label* versionLabel;
	FwGui::ImageButton* closeButton;
};
