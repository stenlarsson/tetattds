#pragma once

#include "control.h"

namespace FwGui
{
	enum StandardImage
	{
		STDIMG_UP = 0,
		STDIMG_DOWN,
		STDIMG_CLOSE,
		STDIMG_BACKSPACE,
		STDIMG_SHIFT
	};
	
	class ImageButton : public Control
	{
	public:
		ImageButton(int x, int y, int width, int height, const uint8_t* bitmap);
		ImageButton(int x, int y, StandardImage image);
		virtual ~ImageButton();
	
		virtual void Paint(Graphics* graphics);
	
	private:
		const uint8_t* bitmap;
	};
}
