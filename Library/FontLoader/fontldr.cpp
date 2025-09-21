#include "fontldr.hpp"
#include "status.hpp"
#include "fileio.hpp"
#include "vga.hpp"
#include "memutil.hpp"
#include "cstring.hpp"

namespace HailOS::FontLoader
{
    Status loadFont(const char* fontFileName, LoadedFontInfo& out)
    {
        API::FileIO::FileObject file;
        Status status = API::FileIO::openFile(fontFileName, file);
        if(status != Status::STATUS_SUCCESS)
        {
            return status;
        }

        BitmapFontHeader hdr = *reinterpret_cast<BitmapFontHeader*>(file.Buffer);
        char* name = reinterpret_cast<char*>(file.Buffer + sizeof(BitmapFontHeader));
        out.DefaultColor = MAKE_RGB(hdr.DefaultRed, hdr.DefaultGreen, hdr.DefaultRed);
        out.FontCharset = hdr.FontCharset;
        out.Height = hdr.FontHeight;
        out.Width = hdr.FontWidth;
        out.FontName = name;
        out.Count = hdr.TotalGlyphCount;
        void* buf = MemoryManager::alloc(out.Height * out.Width * out.Count);
        if(buf == nullptr)
        {
            API::FileIO::closeFile(file);
            return Status::STATUS_MEMORY_ALLOCATION_FAILED;
        }

        MemoryManager::memcopy(buf, file.Buffer + sizeof(BitmapFontHeader) + StdLib::C::strlen(name) + 1, out.Height * out.Width * out.Count + out.Count);

        out.FontData = buf;

        return Status::STATUS_SUCCESS;
    }

    void unloadFont(LoadedFontInfo& font)
    {
        MemoryManager::free(font.FontData, font.Height * font.Width * font.Count+ font.Count);
    }

    const void* getGlyph(char ch, LoadedFontInfo& info)
    {
        for(size_t offset = 0; offset<info.Height * info.Width * info.Count + info.Count; offset += info.Height * info.Width)
        {
            char idx_ch = reinterpret_cast<u8*>(info.FontData)[offset];
            if(idx_ch == ch) [[unlikely]]
            {
                return info.FontData + offset + 1;
            }
        }

        //フォントデータに存在しないコード
        return nullptr;
    }
}