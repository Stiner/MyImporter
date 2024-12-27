#include "PMXTypes.h"

namespace PMX
{
    void Text::SetText(PMX::Byte* const InBuffer, const MemSize InBufferSize, const PMX::Text::EncodingType InEncoding)
    {
        Encoding = InEncoding;

        switch (Encoding)
        {
            case EncodingType::UTF16LE:
                TextData.UTF16LE = reinterpret_cast<wchar_t*>(InBuffer);
                Length = static_cast<int>(InBufferSize / 2);
                break;
            case EncodingType::UTF8:
                TextData.UTF8 = reinterpret_cast<char*>(InBuffer);
                Length = static_cast<int>(InBufferSize / 3);
                break;
            default:
                return;
        }
    }

    void Text::Delete()
    {
        switch (Encoding)
        {
            case PMX::Text::EncodingType::UTF16LE:
                if (TextData.UTF16LE != nullptr)
                    delete[] TextData.UTF16LE;
                break;
            case PMX::Text::EncodingType::UTF8:
                if (TextData.UTF8 != nullptr)
                    delete[] TextData.UTF8;
                break;
        }

        TextData = { 0 };
        Length = 0;
        Encoding = (PMX::Text::EncodingType)0;
    }

    PMX::Text::EncodingType PMX::Text::GetEncodingType()
    {
        return Encoding;
    }

    MemSize PMX::Text::GetBufferSize()
    {
        switch (Encoding)
        {
            case PMX::Text::EncodingType::UTF16LE: return (Length + 1) * 2;
            case PMX::Text::EncodingType::UTF8:    return (Length + 1) * 3;

            default: return 0;
        }
    }

    const wchar_t* Text::GetUTF16LE()
    {
        return TextData.UTF16LE;
    }

    const char* Text::GetUTF8()
    {
        return TextData.UTF8;
    }

    int Text::GetLength()
    {
        return Length;
    }
}