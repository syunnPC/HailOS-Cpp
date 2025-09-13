#include "basetype.hpp"
#include "fileio.hpp"
#include "status.hpp"
#include "vgatype.hpp"
#include "vga.hpp"
#include "bitmap.hpp"
#include "memutil.hpp"

namespace HailOS::Graphic::BitmapImage
{
    constexpr auto BI_RGB = 0;

    Status drawBitmapToBuffer(const char* fileName, Point location, Rectangle* outRect)
    {
        API::FileIO::FileObject file;
        Status status = API::FileIO::openFile(fileName, file);
        if(status != Status::STATUS_SUCCESS)
        {
            return status;
        }

        BitmapFileHeader bf = *reinterpret_cast<BitmapFileHeader*>(file.Buffer);
        BitmapInfoHeader bi = *reinterpret_cast<BitmapInfoHeader*>(reinterpret_cast<size_t>(file.Buffer) + sizeof(BitmapFileHeader));
        if(bf.bfType[0] != 'B' || bf.bfType[1] != 'M' || bi.biCompression != BI_RGB || bi.bcBitCount != 32 || bi.bcHeight <= 0)
        {
            API::FileIO::closeFile(file);
            return Status::STATUS_NOT_IMPLEMENTED;
        }

        i32 height = bi.bcHeight;
        u32 width = bi.bcWidth;

        size_t draw_px = 0;

        for(i32 y=height -1 ; y>=0; y--)
        {
            for(u32 x=0; x<width; x++)
            {
                RGBQuad q = *reinterpret_cast<RGBQuad*>(reinterpret_cast<size_t>(file.Buffer) + bf.bfOffBits + sizeof(RGBQuad) * draw_px++);
                drawPixelToBuffer(COORD(location.X + x, location.Y + y), MAKE_RGB(q.rgbRed, q.rgbGreen, q.rgbBlue));
            }
        }

        if(outRect != nullptr)
        {
            *outRect = RECT(width, height);
        }

        API::FileIO::closeFile(file);
        return Status::STATUS_SUCCESS;
    }

    Status drawBitmap(const char* fileName, Point location, Rectangle* outRect)
    {
        Status status = drawBitmapToBuffer(fileName, location, outRect);
        if(status == Status::STATUS_SUCCESS)
        {
            drawBufferContentsToFrameBuffer();
        }
        else
        {
            setLastStatus(status);
        }
        return status;
    }

    Status convertBitmapToRGBArray(const char* fileName, RGB**& outBuf, Rectangle& outRect)
    {
        API::FileIO::FileObject file;
        Status status = API::FileIO::openFile(fileName, file);
        if (status != Status::STATUS_SUCCESS)
        {
            return status;
        }

        BitmapFileHeader bf = *reinterpret_cast<BitmapFileHeader *>(file.Buffer);
        BitmapInfoHeader bi = *reinterpret_cast<BitmapInfoHeader *>(reinterpret_cast<size_t>(file.Buffer) + sizeof(BitmapFileHeader));
        if (bf.bfType[0] != 'B' || bf.bfType[1] != 'M' || bi.biCompression != BI_RGB || bi.bcBitCount != 32 || bi.bcHeight <= 0)
        {
            API::FileIO::closeFile(file);
            return Status::STATUS_NOT_IMPLEMENTED;
        }

        i32 height = bi.bcHeight;
        u32 width = bi.bcWidth;

        outRect.Width = width;
        outRect.Height = height;
        
        size_t draw_px;

        outBuf = reinterpret_cast<RGB**>(MemoryManager::alloc(sizeof(RGB*) * height));
        for(i32 y=height-1; y<=0; y--)
        {
            outBuf[y] = reinterpret_cast<RGB*>(MemoryManager::alloc((sizeof(RGB) * width)));
            for(u32 x=0; x<width; x++)
            {
                RGBQuad q = *reinterpret_cast<RGBQuad *>(reinterpret_cast<size_t>(file.Buffer) + bf.bfOffBits + sizeof(RGBQuad) * draw_px++);
                outBuf[y][x] = MAKE_RGB(q.rgbRed, q.rgbGreen, q.rgbBlue);
            }
        }

        API::FileIO::closeFile(file);
        return Status::STATUS_SUCCESS;
    }
}