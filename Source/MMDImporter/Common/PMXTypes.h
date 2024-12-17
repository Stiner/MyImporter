#pragma once

namespace PMX
{
    typedef unsigned char   Byte;

#ifdef _WIN64
    typedef unsigned __int64    Size_T;
    typedef __int64             SSize_T;
#else
    typedef unsigned long       Size_T;
    typedef long                SSize_T;
#endif

    struct Vector2
    {
        float X;
        float Y;
    };

    struct Vector3
    {
        float X;
        float Y;
        float Z;
    };

    struct Vector4
    {
        float X;
        float Y;
        float Z;
        float W;
    };
}
