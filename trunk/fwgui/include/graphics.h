#pragma once
#include "font.h"

namespace FwGui
{
	struct Color
	{
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) : r(r), g(g), b(b), a(a) {}
		uint8_t r, g, b, a;
	};
	
	class Graphics
	{
	public:
		Graphics(uint16_t* imagebuffer, int screenWidth, int screenHeight);
	
		void SetPenColor(const Color& color);
		void SetFillColor(const Color& color);
		void SetFont(const Font* font);
		void DrawString(int x, int y, const char* text);
		void DrawCenteredString(int x, int y, int width, const char* text);
		void DrawRect(int x, int y, int width, int height);
		void FillRect(int x, int y, int width, int height);
		void BlitAlpha(int x, int y, int width, int height, const uint8_t* bitmap);
		void Blit(int x, int y, int width, int height, const uint16_t* bitmap);
	
	private:
		uint16_t* screenBuffer;
		int screenWidth;
		int screenHeight;
		const Font* font;
		Color penColor;
		Color fillColor;
	};
}
