﻿#pragma once

#include "PMXTypes.h"

namespace PMX
{
    enum class EncodingType : PMX::Byte
    {
        UTF16LE,
        UTF8,
    };

    struct Text
    {
    public:
        void FromBytes(PMX::Byte* const InBuffer, const size_t InBufferSize, const PMX::EncodingType InEncoding);
        void Delete();

        PMX::EncodingType GetEncodingType();
        const wchar_t* GetUTF16LE();
        const char* GetUTF8();

        int GetLength();

        // Null 끝을 포함한 메모리 크기
        size_t GetBufferSize();

    protected:
        int Length;

        PMX::EncodingType Encoding;

        union
        {
            const wchar_t* UTF16LE;
            const char* UTF8;
        } TextData;
    };
}
