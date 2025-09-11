#include "vgatype.hpp"
#include "vga.hpp"
#include "memutil.hpp"
#include "cursor.hpp"

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
            return false;
        }

        sScreenSize = Graphic::getScreenResolution();
        sPPSL = Graphic::getPixelsPerScanLine();
        sCursorLoc = COORD(0, 0);

        gCursorSize = RECT(8, 16);
        gBufferContentsUnderCursor = reinterpret_cast<Graphic::FrameBufferColor**>(MemoryManager::alloc(gCursorSize.Width * sizeof(Graphic::FrameBufferColor*)));
        gCursorImage = reinterpret_cast<Graphic::FrameBufferColor**>(MemoryManager::alloc(gCursorSize.Width * sizeof(Graphic::FrameBufferColor*)));
        if(gCursorImage == nullptr || gBufferContentsUnderCursor == nullptr)
        {
            MemoryManager::free(gBufferContentsUnderCursor, gCursorSize.Height * sizeof(Graphic::FrameBufferColor*));
            MemoryManager::free(gCursorImage, gCursorSize.Height * sizeof(Graphic::FrameBufferColor*));
            return false;
        }

        for(u64 w=0; w<gCursorSize.Height; w++)
        {
            gBufferContentsUnderCursor[w] = reinterpret_cast<Graphic::FrameBufferColor*>(MemoryManager::allocInitializedMemory(gCursorSize.Width * sizeof(Graphic::FrameBufferColor), 0));
            gCursorImage[w] = reinterpret_cast<Graphic::FrameBufferColor*>(MemoryManager::allocInitializedMemory(gCursorSize.Width * sizeof(Graphic::FrameBufferColor), 0));
            if(gCursorImage[w] == nullptr || gBufferContentsUnderCursor[w] == nullptr)
            {
                //TODO: これまでに確保したメモリの解放処理を実装
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

        updateBufferUnderCursor();
        sInitialized = true;
        return true;
    }

    void updateCursor(Graphic::Point location)
    {
        if(!sInitialized)
        {
            return;
        }

        //動いていない場合
        if(MemoryManager::memeq(&location, &sCursorLoc, sizeof(Graphic::Point)))
        {
            return;
        }

        //カーソルがもともとあった部分のバッファを書き戻す
        u8* buf = reinterpret_cast<u8*>(Graphic::getBufferAddress());
        for (u32 y = sCursorLoc.Y; y < sCursorLoc.Y + gCursorSize.Height; y++)
        {
            for (u32 x = sCursorLoc.X; x < sCursorLoc.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor *>(reinterpret_cast<addr_t>(buf) + CALC_PIXEL_OFFSET(x, y));
                *p = gBufferContentsUnderCursor[y][x];
            }
        }

        //カーソル位置を更新し、カーソルの下のものを保存
        sCursorLoc = location;
        updateBufferUnderCursor();

        for(u32 y=location.Y; y<location.Y + gCursorSize.Height; y++)
        {
            for(u32 x=location.X; x<location.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor *>(reinterpret_cast<addr_t>(buf) + CALC_PIXEL_OFFSET(x, y));
                *p = gCursorImage[y][x];
            }
        }
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

        for(u32 y = sCursorLoc.Y; y < sCursorLoc.Y + gCursorSize.Height; y++)
        {
            for(u32 x = sCursorLoc.X; x<sCursorLoc.X + gCursorSize.Width; x++)
            {
                Graphic::FrameBufferColor* p = reinterpret_cast<Graphic::FrameBufferColor*>(reinterpret_cast<addr_t>(Graphic::getBufferAddress()) + CALC_PIXEL_OFFSET(x, y));
                gBufferContentsUnderCursor[y][x] = *p;
            }
        }
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
}