#include "PMXMeshData.h"
#include "PMXDefines.h"

#include <memory>
#include <cassert>

bool PMXMeshData::LoadBinary(const PMX::Byte* const InBuffer, const size_t InBufferSize)
{
    if (InBuffer == nullptr || InBufferSize == 0)
        return false;

    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadHeader(BufferCur);

    if (IsValidPMXFile(Header) == false)
        return false;

    BufferCur += ReadModelInfo(BufferCur);

    BufferCur += ReadVertices(BufferCur);

    BufferCur += ReadSurfaces(BufferCur);

    BufferCur += ReadTextures(BufferCur);

    // 지금까지 잘 로드 했는지 검사
    if (InBuffer != BufferCur - InBufferSize)
    {
        Delete();
        return false;
    }

    return true;
}

void PMXMeshData::Delete()
{
    ModelInfo.Delete();

    PMX_SAFE_DELETE_ARRAY(Vertices);
    VertexCount = 0;

    PMX_SAFE_DELETE_ARRAY(Surfaces);
    SurfaceCount = 0;

    PMX_SAFE_DELETE_ARRAY(Textures);
    TextureCount = 0;

    PMX_SAFE_DELETE_ARRAY(Materials);
    MaterialCount = 0;

    PMX_SAFE_DELETE_ARRAY(Bones);
    BoneCount = 0;

    PMX_SAFE_DELETE_ARRAY(Morphs);
    MorphCount = 0;

    PMX_SAFE_DELETE_ARRAY(DisplayFrames);
    DisplayFrameCount = 0;

    PMX_SAFE_DELETE_ARRAY(Rigidbodies);
    RigidbodyCount = 0;

    PMX_SAFE_DELETE_ARRAY(Joints);
    JointCount = 0;

    PMX_SAFE_DELETE_ARRAY(SoftBodies);
    SoftBodyCount = 0;
}

size_t PMXMeshData::ReadBuffer(void* OutDest, const PMX::Byte* const InBuffer, const size_t ReadSize)
{
    memcpy(OutDest, InBuffer, ReadSize);
    return ReadSize;
}

size_t PMXMeshData::ReadText(PMX::Text& OutString, const PMX::Byte* InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;
    const PMX::EncodingType Encoding = Header.TextEncoding;

    OutString.Delete();

    int TextBytesSize = 0;
    BufferCur += ReadBuffer(&TextBytesSize, BufferCur, sizeof(TextBytesSize));

    int TextBytesSizeWithNull = 0;
    switch (Encoding)
    {
        case PMX::EncodingType::UTF16LE:
            TextBytesSizeWithNull = TextBytesSize + 2;
            break;

        case PMX::EncodingType::UTF8:
            TextBytesSizeWithNull = TextBytesSize + 3;
            break;

        default:
            return 0;
    }

    PMX::Byte* TextBuffer = new PMX::Byte[TextBytesSizeWithNull]{ 0 };
    BufferCur += ReadBuffer(TextBuffer, BufferCur, sizeof(PMX::Byte) * TextBytesSize);

    OutString.SetText(TextBuffer, TextBytesSize, Encoding);

    return BufferCur - InBuffer;
}

bool PMXMeshData::IsValidPMXFile(const PMX::Header& const Header)
{
    return (Header.Signature[0] == 'P' && Header.Signature[1] == 'M' && Header.Signature[2] == 'X' && Header.Signature[3] == 0x20);
}

size_t PMXMeshData::ReadHeader(const PMX::Byte* const InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadBuffer(Header.Signature, BufferCur, sizeof(Header.Signature));
    BufferCur += ReadBuffer(&Header.Version, BufferCur, sizeof(Header.Version));

    PMX::Byte GlobalsCount = 0;
    BufferCur += ReadBuffer(&GlobalsCount, BufferCur, sizeof(PMX::Byte));

    BufferCur += ReadBuffer(&Header.TextEncoding, BufferCur, sizeof(PMX::Byte) * GlobalsCount);

    return BufferCur - InBuffer;
}

size_t PMXMeshData::ReadModelInfo(const PMX::Byte* const InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadText(ModelInfo.ModelNameLocal,     BufferCur);
    BufferCur += ReadText(ModelInfo.ModelNameUniversal, BufferCur);
    BufferCur += ReadText(ModelInfo.CommentsLocal,      BufferCur);
    BufferCur += ReadText(ModelInfo.CommentsUniversal,  BufferCur);

    return BufferCur - InBuffer;
}

size_t PMXMeshData::ReadVertices(const PMX::Byte* InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadBuffer(&VertexCount, BufferCur, sizeof(VertexCount));

    if (VertexCount <= 0)
        return 0;

    Vertices = new PMX::VertexData[VertexCount]{ 0 };

    for (int i = 0; i < VertexCount; ++i)
    {
        PMX::VertexData& Vertex = Vertices[i];

        const PMX::Byte AdditionalVectorCount = Header.AdditionalVectorCount;
        const PMX::Byte BoneIndexSize         = Header.BoneIndexSize;

        BufferCur += ReadBuffer(&Vertex.Position, BufferCur, sizeof(Vertex.Position));
        BufferCur += ReadBuffer(&Vertex.Normal,   BufferCur, sizeof(Vertex.Normal));
        BufferCur += ReadBuffer(&Vertex.UV,       BufferCur, sizeof(Vertex.UV));

        BufferCur += ReadBuffer(Vertex.Additional, BufferCur, sizeof(Vertex.Additional[0]) * AdditionalVectorCount);

        BufferCur += ReadBuffer(&Vertex.WeightDeformType, BufferCur, sizeof(Vertex.WeightDeformType));

        switch (Vertex.WeightDeformType)
        {
            case PMX::WeightDeformType::BDEF1:
                {
                    auto& WeightDeform = Vertex.WeightDeform.BDef1;

                    BufferCur += ReadBuffer(&WeightDeform.Index0, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                }
                break;
            case PMX::WeightDeformType::BDEF2:
                {
                    auto& WeightDeform = Vertex.WeightDeform.BDef2;

                    BufferCur += ReadBuffer(&WeightDeform.Index0, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);

                    BufferCur += ReadBuffer(&WeightDeform.Weight0, BufferCur, sizeof(WeightDeform.Weight0));
                    WeightDeform.Weight1 = 1.0f - WeightDeform.Weight0;
                }
                break;
            case PMX::WeightDeformType::BDEF4:
                {
                    auto& WeightDeform = Vertex.WeightDeform.BDef4;

                    BufferCur += ReadBuffer(&WeightDeform.Index0, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index2, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index3, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);

                    BufferCur += ReadBuffer(&WeightDeform.Weight0, BufferCur, sizeof(WeightDeform.Weight0));
                    BufferCur += ReadBuffer(&WeightDeform.Weight1, BufferCur, sizeof(WeightDeform.Weight1));
                    BufferCur += ReadBuffer(&WeightDeform.Weight2, BufferCur, sizeof(WeightDeform.Weight2));
                    BufferCur += ReadBuffer(&WeightDeform.Weight3, BufferCur, sizeof(WeightDeform.Weight3));
                }
                break;
            case PMX::WeightDeformType::SDEF:
                {
                    auto& WeightDeform = Vertex.WeightDeform.SDef;

                    BufferCur += ReadBuffer(&WeightDeform.Index0, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);

                    BufferCur += ReadBuffer(&WeightDeform.Weight0, BufferCur, sizeof(WeightDeform.Weight0));
                    WeightDeform.Weight1 = 1.0f - WeightDeform.Weight0;

                    BufferCur += ReadBuffer(&WeightDeform.C, BufferCur, sizeof(WeightDeform.C));
                    BufferCur += ReadBuffer(&WeightDeform.R0, BufferCur, sizeof(WeightDeform.R0));
                    BufferCur += ReadBuffer(&WeightDeform.R1, BufferCur, sizeof(WeightDeform.R1));
                }
                break;
            case PMX::WeightDeformType::QDEF:
                {
                    auto& WeightDeform = Vertex.WeightDeform.QDef;

                    BufferCur += ReadBuffer(&WeightDeform.Index0, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index2, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index3, BufferCur, sizeof(PMX::Byte) * BoneIndexSize);

                    BufferCur += ReadBuffer(&WeightDeform.Weight0, BufferCur, sizeof(WeightDeform.Weight0));
                    BufferCur += ReadBuffer(&WeightDeform.Weight1, BufferCur, sizeof(WeightDeform.Weight1));
                    BufferCur += ReadBuffer(&WeightDeform.Weight2, BufferCur, sizeof(WeightDeform.Weight2));
                    BufferCur += ReadBuffer(&WeightDeform.Weight3, BufferCur, sizeof(WeightDeform.Weight3));
                }
                break;

            default:
                assert(0);
                break;
        }

        BufferCur += ReadBuffer(&Vertex.EdgeScale, BufferCur, sizeof(Vertex.EdgeScale));
    }

    return BufferCur - InBuffer;
}

size_t PMXMeshData::ReadSurfaces(const PMX::Byte* InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;
    const int VertexIndexSize = Header.VertexIndexSize;

    int IndexCount = 0;
    BufferCur += ReadBuffer(&IndexCount, BufferCur, sizeof(IndexCount));

    if (IndexCount <= 0)
        return 0;

    // 3개로 하나의 삼각형 구성
    SurfaceCount = IndexCount / 3;

    Surfaces = new PMX::SurfaceData[SurfaceCount]{ 0 };

    for (int i = 0; i < SurfaceCount; ++i)
    {
        BufferCur += ReadBuffer(&Surfaces[i].VertexIndex[0], BufferCur, sizeof(PMX::Byte) * VertexIndexSize);
        BufferCur += ReadBuffer(&Surfaces[i].VertexIndex[1], BufferCur, sizeof(PMX::Byte) * VertexIndexSize);
        BufferCur += ReadBuffer(&Surfaces[i].VertexIndex[2], BufferCur, sizeof(PMX::Byte) * VertexIndexSize);
    }

    return BufferCur - InBuffer;
}

size_t PMXMeshData::ReadTextures(const PMX::Byte* InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadBuffer(&TextureCount, BufferCur, sizeof(TextureCount));

    if (TextureCount <= 0)
        return 0;

    Textures = new PMX::TextureData[TextureCount];
    memset(Textures, 0, sizeof(PMX::TextureData) * TextureCount);

    for (int i = 0; i < TextureCount; ++i)
    {
        BufferCur += ReadText(Textures[i].Path, BufferCur);
    }

    return BufferCur - InBuffer;
}

size_t PMXMeshData::ReadMaterials(const PMX::Byte* InBuffer)
{
    return size_t();
}

size_t PMXMeshData::ReadBones(const PMX::Byte* InBuffer)
{
    return size_t();
}

size_t PMXMeshData::ReadMorphs(const PMX::Byte* InBuffer)
{
    return size_t();
}

size_t PMXMeshData::ReadDisplayFrames(const PMX::Byte* InBuffer)
{
    return size_t();
}

size_t PMXMeshData::ReadRigidbodies(const PMX::Byte* InBuffer)
{
    return size_t();
}

size_t PMXMeshData::ReadJoints(const PMX::Byte* InBuffer)
{
    return size_t();
}

size_t PMXMeshData::ReadSoftBodies(const PMX::Byte* InBuffer)
{
    return size_t();
}
