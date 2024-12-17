#include "PMXText.h"

void PMX::Text::FromBytes(PMX::Byte* const InBuffer, const size_t InBufferSize, const PMX::EncodingType InEncoding)
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

void PMX::Text::Delete()
{
    switch (Encoding)
    {
        case PMX::EncodingType::UTF16LE:
            if (TextData.UTF16LE != nullptr)
                delete[] TextData.UTF16LE;
            break;
        case PMX::EncodingType::UTF8:
            if (TextData.UTF8 != nullptr)
                delete[] TextData.UTF8;
            break;
    }

    Length = 0;
    Encoding = (PMX::EncodingType)0;
}

PMX::EncodingType PMX::Text::GetEncodingType()
{
    return Encoding;
}

size_t PMX::Text::GetBufferSize()
{
    switch (Encoding)
    {
        case PMX::EncodingType::UTF16LE: return (Length + 1) * 2;
        case PMX::EncodingType::UTF8:    return (Length + 1) * 3;

        default: return 0;
    }
}

const wchar_t* PMX::Text::GetUTF16LE()
{
    return TextData.UTF16LE;
}

const char* PMX::Text::GetUTF8()
{
    return TextData.UTF8;
}

int PMX::Text::GetLength()
{
    return Length;
}
