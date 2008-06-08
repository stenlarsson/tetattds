#pragma once

#include <dialog.h>

namespace FwGui
{
	class Button;
	class Label;
	class ImageButton;
}

class StatusDialog : public FwGui::Dialog
{
public:
	StatusDialog(const char* title);
	virtual ~StatusDialog();

	virtual void ControlClicked(FwGui::Control* control);

	void SetStatus(const char* status);
	
	bool abort;

private:
	FwGui::ImageButton* closeButton;
	FwGui::Label* statusLabel;
};
