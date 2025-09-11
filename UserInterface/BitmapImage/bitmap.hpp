#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"
#include "status.hpp"

namespace HailOS::Graphic::BitmapImage
{
    struct BitmapFileHeader
    {
        char bfType[2];
        u32 bfSize;
        u16 bfReserved1;
        u16 bfReserved2;
        u32 bfOffBits;
    } PACKED;

    struct BitmapInfoHeader
    {
        u32 bcSize;
        u32 bcWidth;
        i32 bcHeight;
        u16 bcPlanes;
        u16 bcBitCount;
        u32 biCompression;
        u32 biSizeImage;
        u32 biXPixPerMeter;
        u32 biYPixPerMeter;
        u32 biClrUsed;
        u32 biClrImportant;
    } PACKED;

    struct RGBQuad
    {
        u8 rgbBlue;
        u8 rgbGreen;
        u8 rgbRed;
        u8 rgbReserved;
    };

    Status drawBitmapToBuffer(const char* fileName, Point location, Rectangle* outRect);
    Status drawBitmap(const char* fileName, Point location, Rectangle* outRect);
    Status convertBitmapToRGBArray(const char* filename, RGB**& outBuf, Rectangle& outRect);
}