#include "vgatype.hpp"
#include "vga.hpp"
#include "memutil.hpp"
#include "cursor.hpp"
#include "kernellib.hpp"
#include "console.hpp"
#include "status.hpp"

#define CALC_PIXEL_OFFSET(x, y) ((y * sPPSL + x) * sizeof(HailOS::Graphic::FrameBufferColor))

namespace HailOS::UI::Cursor
{
    //カーソルのイメージ
    Graphic::FrameBufferColor** gCursorImage = nullptr;
    //カーソルの下にあるバッファの内容を保存し、カーソル移動時に書き戻す
    Graphic::FrameBufferColor** gBufferContentsUnderCursor;
    //カーソルの四角形サイズ（バッファはこの大きさで保存）
    Graphic::Rectangle gCursorSize;
    static Graphic::Point sCursorLoc;
    static u64 sPPSL;
    static Graphic::Rectangle sScreenSize;
    static bool sInitialized = false;

    const u8 gCursor[] =
    {
        0b10000000, 0b11000000, 0b11100000, 0b11110000, 0b11111000, 0b11111100, 0b11111110, 0b11111111, 0b11111000, 0b11011100, 0b10011100, 0b00001110, 0b00001110, 0b00000111, 0b00000111, 0b00000011
    };

    bool initCursor(void)
    {
        /*
            TODO:実装方式
            カーソルのビットマップをgCursor->gCursorImageに展開（サイズ:gCursorSize.Height * gCursorSize.Width）
            カーソルを描画する部分のバッファの内容をgBufferContentsUnderCursorにコピー
            カーソル更新時に位置が変わるなら、gBufferContentsUnderCursorにコピーされたバッファ内容をフレームバッファーに転送し、次カーソルを描画する
            位置のバッファの内容に更新し、カーソルを描画する

            将来的にはBMP画像のロードにも対応→loadCursor()を実装
        */

        if(sInitialized == true)
        {
            return true;
        }

        if(!Graphic::isGraphicAvailable())
        {
            setLastStatus(Status::STATUS_NOT_INITIALIZED);
            return false;
        }

        Kernel::Utility::disableInterrupts();

        sScreenSize = Graphic::getScreenResolution();
        sPPSL = Graphic::getPixelsPerScanLine();
        sCursorLoc = COORD(0, 0);

        gCursorSize = RECT(8, 16);
        gBufferContentsUnderCursor = reinterpret_cast<Graphic::FrameBufferColor**>(MemoryManager::alloc(gCursorSize.Height * sizeof(Graphic::FrameBufferColor*)));
        gCursorImage = reinterpret_cast<Graphic::FrameBufferColor**>(MemoryManager::alloc(gCursorSize.Height * sizeof(Graphic::FrameBufferColor*)));
        if(gCursorImage == nullptr || gBufferContentsUnderCursor == nullptr)
        {
            MemoryManager::free(gBufferContentsUnderCursor, gCursorSize.Height * sizeof(Graphic::FrameBufferColor*));
            MemoryManager::free(gCursorImage, gCursorSize.Height * sizeof(Graphic::FrameBufferColor*));
            setLastStatus(Status::STATUS_MEMORY_ALLOCATION_FAILED);

            Kernel::Utility::enableInterrupts();
            return false;
        }

        for(u64 h=0; h<gCursorSize.Height; h++)
        {
            gBufferContentsUnderCursor[h] = reinterpret_cast<Graphic::FrameBufferColor*>(MemoryManager::allocInitializedMemory(gCursorSize.Width * sizeof(Graphic::FrameBufferColor), 0));
            gCursorImage[h] = reinterpret_cast<Graphic::FrameBufferColor*>(MemoryManager::allocInitializedMemory(gCursorSize.Width * sizeof(Graphic::FrameBufferColor), 0));
            if(gCursorImage[h] == nullptr || gBufferContentsUnderCursor[h] == nullptr)
            {
                //TODO: これまでに確保したメモリの解放処理を実装
                setLastStatus(Status::STATUS_ERROR);
                Kernel::Utility::enableInterrupts();
                return false;
            }
        }

        for(u64 y=0; y<gCursorSize.Height; y++)
        {
            const u8 line = gCursor[y];
            for(u64 x=0; x<gCursorSize.Width; x++)
            {
                if(line & ( 1 << (7 - x) ))
                {
                    gCursorImage[y][x] = Graphic::convertColor(CURSOR_DEFAULT_COLOR);
                }
            }
        }

        sInitialized = true;

        updateBufferUnderCursor();
        drawCursor();

        Kernel::Utility::enableInterrupts();
        return true;
    }

    void updateCursor(Graphic::Point location)
    {
        
        if(!sInitialized)
        {
            return;
        }

        Kernel::Utility::disableInterrupts();

        //カーソルがもともとあった部分のバッファを書き戻す
        u8* buf = reinterpret_cast<u8*>(Graphic::getFrameBufferAddress());
        for (u32 y = sCursorLoc.Y; y < sCursorLoc.Y + gCursorSize.Height; y++)
        {
            for (u32 x = sCursorLoc.X; x < sCursorLoc.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor *>(reinterpret_cast<u8*>(buf) + CALC_PIXEL_OFFSET(x, y));
                u32 ly = y - sCursorLoc.Y;
                u32 lx = x - sCursorLoc.X;
                if(ly < gCursorSize. Height && lx < gCursorSize.Width)
                {
                    if(gCursorImage[ly][lx].Color4 != 0)
                    {
                        *p = gBufferContentsUnderCursor[ly][lx];
                    }
                }
            }
        }

        //カーソル位置を更新し、カーソルの下のものを保存
        sCursorLoc = location;
        updateBufferUnderCursor();

        for(u32 y=location.Y; y<location.Y + gCursorSize.Height; y++)
        {
            for(u32 x=location.X; x<location.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor *>(reinterpret_cast<u8*>(buf) + CALC_PIXEL_OFFSET(x, y));
                u32 ly = y - sCursorLoc.Y;
                u32 lx = x - sCursorLoc.X;
                if (ly < gCursorSize.Height && lx < gCursorSize.Width)
                {
                    if(gCursorImage[ly][lx].Color4 != 0)
                    {
                        *p = gCursorImage[ly][lx];
                    }
                }
            }
        }

        Kernel::Utility::enableInterrupts();
    }

    //コンソールで再描画があった時にも、描画関数側で呼ぶようにする
    void updateBufferUnderCursor()
    {
        /*
            更新されたbufferの中からカーソルの下の部分を更新
        */

        if(!sInitialized)
        {
            return;
        }

        Kernel::Utility::disableInterrupts();

        u8* fb = reinterpret_cast<u8*>(Graphic::getFrameBufferAddress());

        for(u32 y = sCursorLoc.Y; y < sCursorLoc.Y + gCursorSize.Height; y++)
        {
            for(u32 x = sCursorLoc.X; x<sCursorLoc.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor *>(fb + CALC_PIXEL_OFFSET(x, y));
                u32 ly = y - sCursorLoc.Y;
                u32 lx = x - sCursorLoc.X;
                if(ly < gCursorSize.Height && lx < gCursorSize.Width)
                {
                    if(gCursorImage[ly][lx].Color4 != 0)
                    {
                        gBufferContentsUnderCursor[ly][lx] = *p;
                    }
                }
            }
        }

        Kernel::Utility::enableInterrupts();
    }

    bool loadCursor(const Graphic::RGB** source, Graphic::Rectangle size)
    {
        //未実装
        if(source == nullptr)
        {
            return false;
        }

        return false;
    }

    void drawCursor(void)
    {
        Kernel::Utility::disableInterrupts();
        u8* fb = reinterpret_cast<u8 *>(Graphic::getFrameBufferAddress());
        for (u32 y = sCursorLoc.Y; y < sCursorLoc.Y + gCursorSize.Height; y++)
        {
            for (u32 x = sCursorLoc.X; x < sCursorLoc.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor *>(fb + CALC_PIXEL_OFFSET(x, y));
                u32 ly = y - sCursorLoc.Y;
                u32 lx = x - sCursorLoc.X;
                if (ly < gCursorSize.Height && lx < gCursorSize.Width)
                {
                    if(gCursorImage[ly][lx].Color4 != 0)
                    {
                        *p = gCursorImage[ly][lx];
                    }
                }
            }
        }
        Kernel::Utility::enableInterrupts();
    }

    Graphic::Point getCursorPosition(void)
    {
        return sCursorLoc;
    }
}