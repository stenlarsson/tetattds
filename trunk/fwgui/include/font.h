#pragma once
#include "fontfile.h"

namespace FwGui
{
	class Font
	{
	public:
		static Font* GUNSHIP_12;
		static Font* VERA_11;
	
		Font(const void* fontdata);
		int GetHeight() const;
		int GetStringWidth(const char* string) const;
		const Glyph* GetGlyph(int character) const;
		const uint8_t* GetBitmapBuffer(const Glyph* glyph) const;
	
	private:
		const uint8_t* fontdata;
		const FontFileHeader* header;
		const Glyph* glyphs;
	};
}
