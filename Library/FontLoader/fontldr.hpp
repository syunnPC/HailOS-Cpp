#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"
#include "status.hpp"

namespace HailOS::FontLoader
{
    enum class Charset : u8
    {
        ASCII = 0,
    };

    struct BitmapFontHeader
    {
        char Magic[2]; //0xBF 0xFF
        u8 FormatVersion;
        Charset FontCharset;
        u8 FontWidth;
        u8 FontHeight;
        u8 TotalGlyphCount;
        u8 DefaultRed;
        u8 DefaultGreen;
        u8 DefaultBlue;
    } PACKED;

    struct LoadedFontInfo
    {
        void* FontData;
        u8 Height;
        u8 Width;
        u8 Count;
        char* FontName;
        Graphic::RGB DefaultColor;
        Charset FontCharset;
    };

    Status loadFont(const char* fontFileName, LoadedFontInfo& out);
    void unloadFont(LoadedFontInfo& font);
    const void* getGlyph(char ch, LoadedFontInfo& info);
}