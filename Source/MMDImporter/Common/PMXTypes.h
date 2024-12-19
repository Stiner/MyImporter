#pragma once

namespace PMX
{
    typedef char            Byte;
    typedef unsigned char   UByte;

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

    enum class IndexType
    {
        Vertex,
        Texture,
        Material,
        Bone,
        Morph,
        Rigidbody,
    };

    enum class EncodingType : PMX::UByte
    {
        UTF16LE,
        UTF8,
    };

    struct Text
    {
    public:
        void SetText(PMX::Byte* const InBuffer, const size_t InBufferSize, const PMX::EncodingType InEncoding);
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
        PMX::Text NameLocal;
        PMX::Text NameUniversal;
        PMX::Text CommentsLocal;
        PMX::Text CommentsUniversal;

        ~ModelInfo()
        {
            Delete();
        }

        void Delete()
        {
            NameLocal.Delete();
            NameUniversal.Delete();
            CommentsLocal.Delete();
            CommentsUniversal.Delete();
        }
    };

    struct BDEF1
    {
        int Index0;
    };

    struct BDEF2
    {
        int Index0;
        int Index1;
        float Weight0;
        float Weight1; // = 1.0 - Weight1
    };

    struct BDEF4
    {
        int Index0;
        int Index1;
        int Index2;
        int Index3;
        float Weight0; // 총 가중치는 1.0을 보장하지 않음
        float Weight1; // 총 가중치는 1.0을 보장하지 않음
        float Weight2; // 총 가중치는 1.0을 보장하지 않음
        float Weight3; // 총 가중치는 1.0을 보장하지 않음
    };

    // Spherical deform blending
    struct SDEF
    {
        int Index0;
        int Index1;
        float Weight0;
        float Weight1; // = 1.0 - Weight1

        PMX::Vector3 C;  // ???
        PMX::Vector3 R0; // ???
        PMX::Vector3 R1; // ???
    };

    // Dual quaternion deform blending
    struct QDEF
    {
        int Index0;
        int Index1;
        int Index2;
        int Index3;
        float Weight0; // 총 가중치는 1.0을 보장하지 않음
        float Weight1; // 총 가중치는 1.0을 보장하지 않음
        float Weight2; // 총 가중치는 1.0을 보장하지 않음
        float Weight3; // 총 가중치는 1.0을 보장하지 않음
    };

    enum class WeightDeformType : PMX::UByte
    {
        BDEF1,
        BDEF2,
        BDEF4,
        SDEF,
        QDEF
    };

    struct VertexData
    {
        PMX::Vector3 Position;
        PMX::Vector3 Normal;
        PMX::Vector2 UV;

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

    struct SurfaceData
    {
        int VertexIndex[3];
    };

    struct TextureData
    {
        PMX::Text Path;
    };

    // Bit flags
    enum class MaterialFlag : PMX::UByte
    {
        NoCull        = 1 << 0, // 후면 컬링 비활성화
        GroundShadow  = 1 << 1, // 지오메트리에 그림자 투영
        DrawShadow    = 1 << 2, // 그림자 맵에 그리기
        ReceiveShadow = 1 << 3, // 그림자 맵으로 부터 그림자 적용
        HasEdge       = 1 << 4, // 연필? 아웃라인
        VertexColor   = 1 << 5, // 추가 Vector의 첫번째 것으로 정점 색 지정
        PointDrawing  = 1 << 6, // 정점을 점으로 표시
        LineDrawing   = 1 << 7, // 삼각형을 선분으로 표시

        // 둘 다 설정된 경우, 점 그리기 플래그가 선 그리기 플래그보다 우선합니다.
    };

    enum class BlendModeType : PMX::UByte
    {
        Disable,
        Multiply,
        Additive,
        AdditionalVec4,
    };

    enum class ToonReferenceType : PMX::UByte
    {
        Texture,
        Internal
    };

    struct MaterialData
    {
        // 재료에 대한 편리한 이름(보통 일본어)
        PMX::Text NameLocal;

        // 재료에 대한 편리한 이름(보통 영어)
        PMX::Text NameUniversal;

        // RGBA 색상(알파는 반투명 재질을 설정합니다)
        PMX::Vector4 DiffuseColor;

        // 반사광의 RGB 색상
        PMX::Vector3 SpecularColor;

        // 반사 하이라이트의 "크기"
        float SpecularStrength;

        // 재료 그림자의 RGB 색상(빛이 없을 때)
        PMX::Vector3 AmbientColor;

        // 재료 플래그 보기
        PMX::MaterialFlag DrawingFlags;

        // 연필 윤곽선 가장자리의 RGBA 색상(반투명의 경우 알파)
        PMX::Vector4 EdgeColor;

        // 연필 윤곽선 크기(1.0은 약 1픽셀이어야 함)
        float EdgeScale;

        // 인덱스 유형을 참조하세요. 이는 기본적으로 텍스처 테이블에서 가져온 것입니다.
        int TextureIndex;

        // 텍스처 인덱스와 동일하지만 환경 매핑용입니다.
        int EnvironmentIndex;

        // 0 = disabled, 1 = multiply, 2 = additive, 3 = additional vec4
        // additional vec4
        // : 첫 번째 추가 vec4를 사용하여 환경 텍스처를 매핑하고,
        //   텍스처 UV로 X 및 Y 값만 사용합니다.
        //   이는 추가 텍스처 레이어로 매핑됩니다.
        //   이는 첫 번째 추가 vec4의 다른 용도와 충돌할 수 있습니다.
        PMX::BlendModeType EnvironmentBlendMode;

        // 0 = Texture reference, 1 = internal reference
        PMX::ToonReferenceType ToonReference;

        // 동작은 Toon 참조 값에 따라 달라집니다.
        // : Toon 참조 바이트가 1인 경우를 제외하고는
        //   Toon 값은 표준 텍스처 및 환경 텍스처 인덱스와 매우 유사한 텍스처 인덱스가 됩니다.
        //   1인 경우 Toon 값은 10개의 내부 Toon 텍스처 세트를 참조하는 바이트가 됩니다
        //   (대부분 구현은 "toon01.bmp" ~ "toon10.bmp"를 내부 텍스처로 사용합니다. 위의 텍스처에 대해 예약된 이름 참조).
        PMX::Byte ToonValue;

        // 스크립팅이나 추가 데이터에 사용됩니다.
        PMX::Text MetaData;

        // 이 재료가 영향을 미치는 표면의 수
        // : 표면 수는 항상 3의 배수입니다.
        //   이는 이전 재료의 오프셋에서 현재 재료의 크기에 따라 결정됩니다.
        //   모든 재료의 표면 수를 모두 더하면 총 표면 수가 됩니다.
        int SurfaceCount;
    };

    struct BoneData
    {
    };

    struct MorphData
    {
    };

    struct DisplayFrameData
    {
    };

    struct RigidbodyData
    {
    };

    struct JointData
    {
    };

    struct SoftBodyData
    {
    };
}
