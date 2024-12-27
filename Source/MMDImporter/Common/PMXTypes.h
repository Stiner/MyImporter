#pragma once

#include "PMXDefines.h"

namespace PMX
{
    typedef char            Byte;
    typedef unsigned char   UByte;

    typedef char            Int8;
    typedef short           Int16;

    typedef unsigned char   UInt8;
    typedef unsigned short  UInt16;

    typedef size_t          MemSize;

    struct Vector2
    {
        float X = 0;
        float Y = 0;
    };

    struct Vector3
    {
        float X = 0;
        float Y = 0;
        float Z = 0;
    };

    struct Vector4
    {
        float X = 0;
        float Y = 0;
        float Z = 0;
        float W = 0;
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

    class Text
    {
    public:
        enum EncodingType : UInt8
        {
            UTF16LE,
            UTF8,
        };

    public:
        void SetText(Byte* const InBuffer, const MemSize InBufferSize, const Text::EncodingType InEncoding);
        void Delete();

        Text::EncodingType GetEncodingType();
        const wchar_t* GetUTF16LE();
        const char* GetUTF8();

        int GetLength();

        // Null 끝을 포함한 메모리 크기
        MemSize GetBufferSize();

    protected:
        int Length = 0;

        Text::EncodingType Encoding = Text::EncodingType::UTF16LE;

        union
        {
            const wchar_t* UTF16LE;
            const char* UTF8;
        } TextData = { 0 };
    };

    struct Header
    {
        UInt8 Signature[4]{ 0 };    // "PMX " (0x50, 0x4d, 0x58, 0x20) : 공백으로 끝을 알림
        float Version = 0;              // 2.0 / 2.1 : floating point 로 비교

        Text::EncodingType TextEncoding = Text::EncodingType::UTF16LE;
        UInt8 AdditionalVectorCount = 0;
        UInt8 VertexIndexSize = 0;
        UInt8 TextureIndexSize = 0;
        UInt8 MaterialIndexSize = 0;
        UInt8 BoneIndexSize = 0;
        UInt8 MorphIndexSize = 0;
        UInt8 RigidbodyIndexSize = 0;
    };

    struct ModelInfo
    {
        Text NameLocal;
        Text NameUniversal;
        Text CommentsLocal;
        Text CommentsUniversal;

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

    struct VertexData
    {
        Vector3 Position;
        Vector3 Normal;
        Vector2 UV;

        // Globals에 의해 갯수가 정해지며, 0개일 수 있음
        Vector4 Additional[4];

        enum class WeightDeformType : UByte
        {
            BDEF1,
            BDEF2,
            BDEF4,
            SDEF,
            QDEF
        } DeformType = (WeightDeformType)-1;

        struct WeightDeform {}* Deform = nullptr;

        // WeightDeformType에 따라 BDEF1..4/SDEF/QDEF 중 택1
        struct BDEF1 : WeightDeform
        {
            int BoneIndex0 = -1;
        };

        struct BDEF2 : WeightDeform
        {
            int BoneIndex0 = -1;
            int BoneIndex1 = -1;
            float Weight0 = -1;
            float Weight1 = -1; // = 1.0 - Weight1
        };

        struct BDEF4 : WeightDeform
        {
            int BoneIndex0 = -1;
            int BoneIndex1 = -1;
            int BoneIndex2 = -1;
            int BoneIndex3 = -1;
            float Weight0 = -1; // 총 가중치는 1.0을 보장하지 않음
            float Weight1 = -1; // 총 가중치는 1.0을 보장하지 않음
            float Weight2 = -1; // 총 가중치는 1.0을 보장하지 않음
            float Weight3 = -1; // 총 가중치는 1.0을 보장하지 않음
        };

        // Spherical deform blending
        struct SDEF : WeightDeform
        {
            int BoneIndex0 = -1;
            int BoneIndex1 = -1;
            float Weight0 = -1;
            float Weight1 = -1; // = 1.0 - Weight1

            Vector3 C;  // ???
            Vector3 R0; // ???
            Vector3 R1; // ???
        };

        // Dual quaternion deform blending
        struct QDEF : WeightDeform
        {
            int BoneIndex0 = -1;
            int BoneIndex1 = -1;
            int BoneIndex2 = -1;
            int BoneIndex3 = -1;
            float Weight0 = -1; // 총 가중치는 1.0을 보장하지 않음
            float Weight1 = -1; // 총 가중치는 1.0을 보장하지 않음
            float Weight2 = -1; // 총 가중치는 1.0을 보장하지 않음
            float Weight3 = -1; // 총 가중치는 1.0을 보장하지 않음
        };

        float EdgeScale = 0;

        ~VertexData()
        {
            PMX_SAFE_DELETE(Deform);
        }
    };

    struct SurfaceData
    {
        int VertexIndex[3]{ 0 };
    };

    struct TextureData
    {
        Text Path;
    };

    struct MaterialData
    {
        // 재료에 대한 편리한 이름(보통 일본어)
        Text NameLocal;

        // 재료에 대한 편리한 이름(보통 영어)
        Text NameUniversal;

        // RGBA 색상(알파는 반투명 재질을 설정합니다)
        Vector4 DiffuseColor;

        // 반사광의 RGB 색상
        Vector3 SpecularColor;

        // 반사 하이라이트의 "크기"
        float SpecularStrength = 0;

        // 재료 그림자의 RGB 색상(빛이 없을 때)
        Vector3 AmbientColor;

        // 재료 플래그 보기
        enum class Flag : UInt8 // Bit flag
        {
            NoCull        = 1 << 0, // 후면 컬링 비활성화
            GroundShadow  = 1 << 1, // 지오메트리에 그림자 투영
            DrawShadow    = 1 << 2, // 그림자 맵에 그리기
            ReceiveShadow = 1 << 3, // 그림자 맵으로 부터 그림자 적용
            HasEdge       = 1 << 4, // 연필? 아웃라인
            VertexColor   = 1 << 5, // 추가 Vector의 첫번째 것으로 정점 색 지정
            PointDrawing  = 1 << 6, // 정점을 점으로 표시
            LineDrawing   = 1 << 7, // 삼각형을 선분으로 표시

            // PointDrawing/LineDrawing 둘 다 설정된 경우, 점 그리기 플래그가 선 그리기 플래그보다 우선합니다.
        } DrawingFlags = (Flag)0;

        // 연필 윤곽선 가장자리의 RGBA 색상(반투명의 경우 알파)
        Vector4 EdgeColor;

        // 연필 윤곽선 크기(1.0은 약 1픽셀이어야 함)
        float EdgeScale = 0;

        // 인덱스 유형을 참조하세요. 이는 기본적으로 텍스처 테이블에서 가져온 것입니다.
        int TextureIndex = 0;

        // 텍스처 인덱스와 동일하지만 환경 매핑용입니다.
        int EnvironmentTextureIndex = 0;

        // 0 = disabled, 1 = multiply, 2 = additive, 3 = additional vec4
        // additional vec4
        // : 첫 번째 추가 vec4를 사용하여 환경 텍스처를 매핑하고,
        //   텍스처 UV로 X 및 Y 값만 사용합니다.
        //   이는 추가 텍스처 레이어로 매핑됩니다.
        //   이는 첫 번째 추가 vec4의 다른 용도와 충돌할 수 있습니다.
        enum class BlendModeType : UInt8
        {
            Disable,
            Multiply,
            Additive,
            AdditionalVec4,
        } EnvironmentBlendMode = (BlendModeType)-1;

        // 0 = Texture reference, 1 = internal reference
        enum class ToonReferenceType : UInt8
        {
            Texture,
            Internal
        } ToonReference = (ToonReferenceType)-1;

        // 동작은 Toon 참조 값에 따라 달라집니다.
        // : Toon 참조 바이트가 1인 경우를 제외하고는
        //   Toon 값은 표준 텍스처 및 환경 텍스처 인덱스와 매우 유사한 텍스처 인덱스가 됩니다.
        //   1인 경우 Toon 값은 10개의 내부 Toon 텍스처 세트를 참조하는 바이트가 됩니다
        //   (대부분 구현은 "toon01.bmp" ~ "toon10.bmp"를 내부 텍스처로 사용합니다. 위의 텍스처에 대해 예약된 이름 참조).
        UInt8 ToonValue = 0;

        // 스크립팅이나 추가 데이터에 사용됩니다.
        Text MetaData;

        // 이 재료가 영향을 미치는 표면의 수
        // : 표면 수는 항상 3의 배수입니다.
        //   이는 이전 재료의 오프셋에서 현재 재료의 크기에 따라 결정됩니다.
        //   모든 재료의 표면 수를 모두 더하면 총 표면 수가 됩니다.
        int SurfaceCount = 0;
    };

    struct BoneData
    {
        Text NameLocal;
        Text NameUniversal;
        Vector3 Position;
        int ParentBoneIndex = 0;
        int Layer = 0;

        enum Flag : UInt16
        {
            // 낮은 8Bit
            IndexedTailPosition  = 1 <<  0,  // 꼬리 위치가 vec3인지 뼈 인덱스인지
            Rotatable            = 1 <<  1,  // 회전을 활성화합니다
            Translatable         = 1 <<  2,  // 번역(전단)이 가능합니다.
            IsVisible            = 1 <<  3,  // ???
            Enabled              = 1 <<  4,  // ???
            UseIK                = 1 <<  5,  // 역 운동학(물리학) 사용
            //                   = 1 <<  6,  // 안씀
            //                   = 1 <<  7,  // 안씀

            // 높은 8bit
            InheritRotation      = 1 <<  8,  // 회전은 다른 뼈에서 상속됩니다.
            InheritTranslation   = 1 <<  9,  // 번역은 다른 뼈대에서 상속됩니다
            FixedAxis            = 1 << 10,  // 뼈의 축은 방향으로 고정되어 있습니다
            LocalCoordinate      = 1 << 11,  // ???
            PhysicsAfterDeform   = 1 << 12,  // ???
            ExternalParentDeform = 1 << 13,  // ???
            //                   = 1 << 14,  // 안씀
            //                   = 1 << 15,  // 안씀
        } Flags = (Flag)0;

        union
        {
            int BoneIndex;
            Vector3 Vector3;
        } TailPositionData = { 0 };

        // InheritRotation/InheritTranslation 플래그 중 하나가 설정된 경우 사용됩니다.
        struct InheritBone
        {
            int ParentBoneIndex = 0;
            float ParentInfluence = 0;
        }* InheritBoneData = nullptr;

        // FixedAxis 플래그가 설정된 경우 사용됩니다.
        struct FixedAxis
        {
            Vector3 AxisDirection;
        }* FixedAxisData = nullptr;

        // LocalCoordinate 플래그가 설정된 경우 사용됩니다.
        struct LocalCoordinate
        {
            Vector3 XVector;
            Vector3 ZVector;
        }* LocalCoordinateData = nullptr;

        // ExternalParentDeform 플래그가 설정된 경우 사용됩니다.
        struct ExternalParent
        {
            int ParentBoneIndex = 0;
        }* ExternalParentData = nullptr;

        // IK 플래그가 설정된 경우 사용됩니다.
        struct IK
        {
            int TargetIndex = 0;
            int LoopCount = 0;
            float LimitRadian = 0;
            int LinkCount = 0;

            struct LinkData
            {
                int BoneIndex = 0;
                Byte HasLimit = 0; // 1과 같으면 각도 제한을 사용합니다.

                struct Limit
                {
                    Vector3 Min;
                    Vector3 Max;
                } LimitData = { 0 };
            }* LinksArray = nullptr;
        } IKData = { 0 };

        ~BoneData()
        {
            PMX_SAFE_DELETE_ARRAY(IKData.LinksArray);
        }
    };

    struct MorphData
    {
        Text NameLocal;
        Text NameUniversal;

        //　뭔지 모르겠음. 힌트 : https://gist.github.com/felixjones/f8a06bd48f9da9a4539f?permalink_comment_id=2751718#gistcomment-2751718
        UInt8 PanelType = 0;

        enum class MorphType : UInt8
        {
            Group,
            Vertex,
            Bone,
            UV,
            AdditionalUV1,
            AdditionalUV2,
            AdditionalUV3,
            AdditionalUV4,
            Material,
            Flip,
            Impulse,
        } Type = (MorphType)-1;

        int OffsetCount = 0;

        struct OffsetBase
        {
        }* Offsets = nullptr;

        struct OffsetGroup : OffsetBase
        {
            int MorphIndex = 0;
            float Rate = 0;
        };

        struct OffsetVertex : OffsetBase
        {
            int VertexIndex = 0;
            Vector3 PositionOffset;
        };

        struct OffsetBone : OffsetBase
        {
            int BoneIndex = 0;
            Vector3 MoveValue;
            Vector4 RotationValue;
        };

        // UV, AddtionalUV1~4 공통
        struct OffsetUV : OffsetBase
        {
            int VertexIndex = 0;
            Vector4 UVOffset;
        };

        struct OffsetMaterial : OffsetBase
        {
            int MaterialIndex = 0;

            enum class MethodType : UInt8
            {
                Multiply,
                Additive
            } OffsetMethod;

            Vector4 DiffuseColor;
            Vector3 SpecularColor;
            float Specularity = 0;
            Vector3 AmbientColor;
            Vector4 EdgeColor;
            float EdgeSize = 0;
            Vector4 TextureTint;
            Vector4 EnvironmentTint;
            Vector4 ToonTint;
        };

        struct OffsetFlip : OffsetBase
        {
            int MorphIndex = 0;
            float Influence = 0;
        };

        struct OffsetImpulse : OffsetBase
        {
            int RigidbodyIndex = 0;
            UInt8 LocalFlag = 0;
            Vector3 MovementSpeed;
            Vector3 RotationTorque;
        };

        ~MorphData()
        {
            PMX_SAFE_DELETE_ARRAY(Offsets);
        }
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
