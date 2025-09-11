#pragma once

#include "basetype.hpp"

#define COORD(x, y) ((HailOS::Graphic::Point){x, y})
#define RECT(x, y) ((HailOS::Graphic::Rectangle){x, y})

namespace HailOS::Graphic
{
    constexpr auto PIXEL_SIZE = 4;

    enum class Direction
    {
        HORIZONTAL_RIGHT = 1,
        HORIZONTAL_LEFT = 2,
        VERTICAL_UP = 4,
        VERITCAL_DOWN = 8
    };

    struct RGB
    {
        u8 Red;
        u8 Green;
        u8 Blue;
    } PACKED;

    struct FrameBufferColor
    {
        u8 Color1;
        u8 Color2;
        u8 Color3;
        u8 Color4;
    } PACKED;

    struct Point
    {
        u64 X;
        u64 Y;
    };

    struct Rectangle
    {
        u64 Width;
        u64 Height;
    };

    enum class PixelFormat
    {
        BGR,
        RGB,
        INVALID,
    };

    struct GraphicInfo
    {
        addr_t FrameBufferBase;
        size_t FrameBufferSize;
        u32 PixelsPerScanLine;
        u32 HorizontalResolution;
        u32 VerticalResolution;
        PixelFormat DisplayPixelFormat;
    } PACKED;

    static_assert(sizeof(FrameBufferColor) == 4);
}