#pragma once

#include "PMXTypes.h"
#include "PMXText.h"

namespace PMX
{
    struct Header
    {
        PMX::Byte Signature[4];     // "PMX " (0x50, 0x4d, 0x58, 0x20) : 공백으로 끝을 알림
        float Version;              // 2.0 / 2.1 : floating point 로 비교

        PMX::EncodingType TextEncoding;
        PMX::Byte AdditionalVectorCount;
        PMX::Byte VertexIndexSize;
        PMX::Byte TextureIndexSize;
        PMX::Byte MaterialIndexSize;
        PMX::Byte BoneIndexSize;
        PMX::Byte MorphIndexSize;
        PMX::Byte RigidbodyIndexSize;
    };

    struct ModelInfo
    {
        PMX::Text ModelNameLocal;
        PMX::Text ModelNameUniversal;
        PMX::Text CommentsLocal;
        PMX::Text CommentsUniversal;

        ~ModelInfo()
        {
            Delete();
        }

        void Delete()
        {
            ModelNameLocal.Delete();
            ModelNameUniversal.Delete();
            CommentsLocal.Delete();
            CommentsUniversal.Delete();
        }
    };

    struct BDEF1
    {
        int Index;
    };

    struct BDEF2
    {
        int Index1;
        int Index2;
        float Weight1;
        float Weight2; // = 1.0 - Weight1
    };

    struct BDEF4
    {
        int Index1;
        int Index2;
        int Index3;
        int Index4;
        float Weight1; // 총 가중치는 1.0을 보장하지 않음
        float Weight2; // 총 가중치는 1.0을 보장하지 않음
        float Weight3; // 총 가중치는 1.0을 보장하지 않음
        float Weight4; // 총 가중치는 1.0을 보장하지 않음
    };

    // Spherical deform blending
    struct SDEF
    {
        int Index1;
        int Index2;
        float Weight1;
        float Weight2; // = 1.0 - Weight1

        PMX::Vector3 C;  // ???
        PMX::Vector3 R0; // ???
        PMX::Vector3 R1; // ???
    };

    // Dual quaternion deform blending
    struct QDEF
    {
        int Index1;
        int Index2;
        int Index3;
        int Index4;
        float Weight1; // 총 가중치는 1.0을 보장하지 않음
        float Weight2; // 총 가중치는 1.0을 보장하지 않음
        float Weight3; // 총 가중치는 1.0을 보장하지 않음
        float Weight4; // 총 가중치는 1.0을 보장하지 않음
    };

    enum class WeightDeformType : PMX::Byte
    {
        BDEF1,
        BDEF2,
        BDEF4,
        SDEF,
        QDEF
    };

    struct Vertex
    {
        PMX::Vector3 Position;
        PMX::Vector3 Normal;
        PMX::Vector3 UV;

        PMX::Vector4 Additional[4];  // Globals에 의해 갯수가 정해지며, 0개일 수 있음

        PMX::WeightDeformType WeightDeformType; // WeightDeformType에 따라 BDEF1..4/SDEF/QDEF 선택
        union
        {
            BDEF1 BDef1;
            BDEF2 BDef2;
            BDEF4 BDef4;
            SDEF SDef;
            QDEF QDef;

        } WeightDeform;

        float EdgeScale;
    };
}