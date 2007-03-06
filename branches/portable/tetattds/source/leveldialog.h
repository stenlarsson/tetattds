#pragma once

#include <dialog.h>

#define NUM_LEVELS 10

namespace FwGui
{
	class Button;
	class ImageButton;
}

class LevelDialog : public FwGui::Dialog
{
public:
	LevelDialog(int defaultLevel);
	virtual ~LevelDialog();

	virtual void ControlClicked(FwGui::Control* control);

	int level;

private:
	FwGui::Button* buttons[NUM_LEVELS];
	FwGui::ImageButton* closeButton;
};
