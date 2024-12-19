#include "PMXMeshData.h"
#include "PMXDefines.h"

#include <memory>
#include <cassert>

void ReadBuffer(void* OutDest, const PMX::Byte*& InOutBufferCursor, const size_t ReadSize)
{
    memcpy(OutDest, InOutBufferCursor, ReadSize);

    InOutBufferCursor += ReadSize;
}

void ReadIndex(int& OutIndex, const PMX::Byte*& InOutBufferCursor, const PMX::IndexType InIndexType, const PMX::Byte InIndexSize)
{
    assert(InIndexSize == 1 || InIndexSize == 2 || InIndexSize == 4);

    OutIndex = -1;
    ReadBuffer(&OutIndex, InOutBufferCursor, InIndexSize);

    if (InIndexSize == 1)
    {
        if (InIndexType == PMX::IndexType::Vertex)
            OutIndex = (PMX::UByte)OutIndex;
        else
            OutIndex = (PMX::Byte)OutIndex;
    }

    if (InIndexSize == 2)
    {
        if (InIndexType == PMX::IndexType::Vertex)
            OutIndex = (unsigned short)OutIndex;
        else
            OutIndex = (short)OutIndex;
    }
}

const PMX::Byte* p;

bool PMXMeshData::LoadBinary(const PMX::Byte* const InBuffer, const size_t InBufferSize)
{
    if (InBuffer == nullptr || InBufferSize == 0)
        return false;

    const PMX::Byte* BufferCur = InBuffer;
    p = BufferCur;

    ReadHeader(BufferCur);

    if (IsValidPMXFile(Header) == false)
        return false;

    ReadModelInfo(BufferCur);

    ReadVertices(BufferCur);

    ReadSurfaces(BufferCur);

    ReadTextures(BufferCur);

    ReadMaterials(BufferCur);

    ReadBones(BufferCur);

    ReadMorphs(BufferCur);

    ReadDisplayFrames(BufferCur);

    ReadRigidbodies(BufferCur);

    ReadJoints(BufferCur);

    ReadSoftBodies(BufferCur);

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

void PMXMeshData::ReadText(PMX::Text& OutString, const PMX::Byte*& InOutBufferCursor)
{
    const PMX::EncodingType Encoding = Header.TextEncoding;

    OutString.Delete();

    int TextBytesSize = 0;
    ReadBuffer(&TextBytesSize, InOutBufferCursor, sizeof(TextBytesSize));

    if (TextBytesSize == 0)
        return;

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
            return;
    }

    PMX::Byte* TextBuffer = new PMX::Byte[TextBytesSizeWithNull]{ 0 };
    ReadBuffer(TextBuffer, InOutBufferCursor, TextBytesSize);

    OutString.SetText(TextBuffer, TextBytesSize, Encoding);
}

bool PMXMeshData::IsValidPMXFile(const PMX::Header& Header)
{
    return (Header.Signature[0] == 'P' && Header.Signature[1] == 'M' && Header.Signature[2] == 'X' && Header.Signature[3] == 0x20);
}

void PMXMeshData::ReadHeader(const PMX::Byte*& InOutBufferCursor)
{
    ReadBuffer(Header.Signature, InOutBufferCursor, sizeof(Header.Signature));
    ReadBuffer(&Header.Version, InOutBufferCursor, sizeof(Header.Version));

    PMX::Byte GlobalsCount = 0;
    ReadBuffer(&GlobalsCount, InOutBufferCursor, sizeof(PMX::Byte));

    ReadBuffer(&Header.TextEncoding, InOutBufferCursor, GlobalsCount);
}

void PMXMeshData::ReadModelInfo(const PMX::Byte*& InOutBufferCursor)
{
    ReadText(ModelInfo.NameLocal, InOutBufferCursor);
    ReadText(ModelInfo.NameUniversal, InOutBufferCursor);
    ReadText(ModelInfo.CommentsLocal, InOutBufferCursor);
    ReadText(ModelInfo.CommentsUniversal, InOutBufferCursor);
}

void PMXMeshData::ReadVertices(const PMX::Byte*& InOutBufferCursor)
{
    ReadBuffer(&VertexCount, InOutBufferCursor, sizeof(VertexCount));

    if (VertexCount <= 0)
        return;

    Vertices = new PMX::VertexData[VertexCount]{ 0 };

    for (int i = 0; i < VertexCount; ++i)
    {
        PMX::VertexData& Vertex = Vertices[i];

        ReadBuffer(&Vertex.Position, InOutBufferCursor, sizeof(Vertex.Position));
        ReadBuffer(&Vertex.Normal, InOutBufferCursor, sizeof(Vertex.Normal));
        ReadBuffer(&Vertex.UV, InOutBufferCursor, sizeof(Vertex.UV));

        ReadBuffer(Vertex.Additional, InOutBufferCursor, sizeof(Vertex.Additional[0]) * Header.AdditionalVectorCount);

        ReadBuffer(&Vertex.WeightDeformType, InOutBufferCursor, sizeof(Vertex.WeightDeformType));

        switch (Vertex.WeightDeformType)
        {
            case PMX::WeightDeformType::BDEF1:
                {
                    auto& WeightDeform = Vertex.WeightDeform.BDef1;

                    ReadIndex(WeightDeform.Index0, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                }
                break;
            case PMX::WeightDeformType::BDEF2:
                {
                    auto& WeightDeform = Vertex.WeightDeform.BDef2;

                    ReadIndex(WeightDeform.Index0, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index1, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);

                    ReadBuffer(&WeightDeform.Weight0, InOutBufferCursor, sizeof(WeightDeform.Weight0));
                    WeightDeform.Weight1 = 1.0f - WeightDeform.Weight0;
                }
                break;
            case PMX::WeightDeformType::BDEF4:
                {
                    auto& WeightDeform = Vertex.WeightDeform.BDef4;

                    ReadIndex(WeightDeform.Index0, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index1, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index2, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index3, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);

                    ReadBuffer(&WeightDeform.Weight0, InOutBufferCursor, sizeof(WeightDeform.Weight0));
                    ReadBuffer(&WeightDeform.Weight1, InOutBufferCursor, sizeof(WeightDeform.Weight1));
                    ReadBuffer(&WeightDeform.Weight2, InOutBufferCursor, sizeof(WeightDeform.Weight2));
                    ReadBuffer(&WeightDeform.Weight3, InOutBufferCursor, sizeof(WeightDeform.Weight3));
                }
                break;
            case PMX::WeightDeformType::SDEF:
                {
                    auto& WeightDeform = Vertex.WeightDeform.SDef;

                    ReadIndex(WeightDeform.Index0, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index1, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);

                    ReadBuffer(&WeightDeform.Weight0, InOutBufferCursor, sizeof(WeightDeform.Weight0));
                    WeightDeform.Weight1 = 1.0f - WeightDeform.Weight0;

                    ReadBuffer(&WeightDeform.C, InOutBufferCursor, sizeof(WeightDeform.C));
                    ReadBuffer(&WeightDeform.R0, InOutBufferCursor, sizeof(WeightDeform.R0));
                    ReadBuffer(&WeightDeform.R1, InOutBufferCursor, sizeof(WeightDeform.R1));
                }
                break;
            case PMX::WeightDeformType::QDEF:
                {
                    auto& WeightDeform = Vertex.WeightDeform.QDef;

                    ReadIndex(WeightDeform.Index0, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index1, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index2, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);
                    ReadIndex(WeightDeform.Index3, InOutBufferCursor, PMX::IndexType::Bone, Header.BoneIndexSize);

                    ReadBuffer(&WeightDeform.Weight0, InOutBufferCursor, sizeof(WeightDeform.Weight0));
                    ReadBuffer(&WeightDeform.Weight1, InOutBufferCursor, sizeof(WeightDeform.Weight1));
                    ReadBuffer(&WeightDeform.Weight2, InOutBufferCursor, sizeof(WeightDeform.Weight2));
                    ReadBuffer(&WeightDeform.Weight3, InOutBufferCursor, sizeof(WeightDeform.Weight3));
                }
                break;
        }

        ReadBuffer(&Vertex.EdgeScale, InOutBufferCursor, sizeof(Vertex.EdgeScale));
    }
}

void PMXMeshData::ReadSurfaces(const PMX::Byte*& InOutBufferCursor)
{
    int IndexCount = 0;
    ReadBuffer(&IndexCount, InOutBufferCursor, sizeof(IndexCount));

    if (IndexCount <= 0)
        return;

    // 3개로 하나의 삼각형 구성
    SurfaceCount = IndexCount / 3;

    Surfaces = new PMX::SurfaceData[SurfaceCount]{ 0 };

    for (int i = 0; i < SurfaceCount; ++i)
    {
        ReadIndex(Surfaces[i].VertexIndex[0], InOutBufferCursor, PMX::IndexType::Vertex, Header.VertexIndexSize);
        ReadIndex(Surfaces[i].VertexIndex[1], InOutBufferCursor, PMX::IndexType::Vertex, Header.VertexIndexSize);
        ReadIndex(Surfaces[i].VertexIndex[2], InOutBufferCursor, PMX::IndexType::Vertex, Header.VertexIndexSize);
    }
}

void PMXMeshData::ReadTextures(const PMX::Byte*& InOutBufferCursor)
{
    ReadBuffer(&TextureCount, InOutBufferCursor, sizeof(TextureCount));

    if (TextureCount <= 0)
        return;

    Textures = new PMX::TextureData[TextureCount];
    memset(Textures, 0, sizeof(PMX::TextureData) * TextureCount);

    for (int i = 0; i < TextureCount; ++i)
    {
        ReadText(Textures[i].Path, InOutBufferCursor);
    }
}

void PMXMeshData::ReadMaterials(const PMX::Byte*& InOutBufferCursor)
{
    ReadBuffer(&MaterialCount, InOutBufferCursor, sizeof(MaterialCount));

    if (MaterialCount <= 0)
        return;

    Materials = new PMX::MaterialData[MaterialCount];
    memset(Materials, 0, sizeof(PMX::MaterialData) * MaterialCount);

    for (int i = 0; i < MaterialCount; ++i)
    {
        PMX::MaterialData& MaterialData = Materials[i];

        ReadText(MaterialData.NameLocal, InOutBufferCursor);
        ReadText(MaterialData.NameUniversal, InOutBufferCursor);
        ReadBuffer(&MaterialData.DiffuseColor, InOutBufferCursor, sizeof(MaterialData.DiffuseColor));
        ReadBuffer(&MaterialData.SpecularColor, InOutBufferCursor, sizeof(MaterialData.SpecularColor));
        ReadBuffer(&MaterialData.SpecularStrength, InOutBufferCursor, sizeof(MaterialData.SpecularStrength));
        ReadBuffer(&MaterialData.AmbientColor, InOutBufferCursor, sizeof(MaterialData.AmbientColor));
        ReadBuffer(&MaterialData.DrawingFlags, InOutBufferCursor, sizeof(MaterialData.DrawingFlags));
        ReadBuffer(&MaterialData.EdgeColor, InOutBufferCursor, sizeof(MaterialData.EdgeColor));
        ReadBuffer(&MaterialData.EdgeScale, InOutBufferCursor, sizeof(MaterialData.EdgeScale));
        ReadIndex(MaterialData.TextureIndex, InOutBufferCursor, PMX::IndexType::Texture, Header.TextureIndexSize);
        ReadIndex(MaterialData.EnvironmentIndex, InOutBufferCursor, PMX::IndexType::Texture, Header.TextureIndexSize);
        ReadBuffer(&MaterialData.EnvironmentBlendMode, InOutBufferCursor, sizeof(MaterialData.EnvironmentBlendMode));
        ReadBuffer(&MaterialData.ToonReference, InOutBufferCursor, sizeof(MaterialData.ToonReference));
        ReadBuffer(&MaterialData.ToonValue, InOutBufferCursor, sizeof(MaterialData.ToonValue));
        ReadText(MaterialData.MetaData, InOutBufferCursor);
        ReadBuffer(&MaterialData.SurfaceCount, InOutBufferCursor, sizeof(MaterialData.SurfaceCount));
    }
}

void PMXMeshData::ReadBones(const PMX::Byte*& InOutBufferCursor)
{



}

void PMXMeshData::ReadMorphs(const PMX::Byte*& InOutBufferCursor)
{



}

void PMXMeshData::ReadDisplayFrames(const PMX::Byte*& InOutBufferCursor)
{



}

void PMXMeshData::ReadRigidbodies(const PMX::Byte*& InOutBufferCursor)
{



}

void PMXMeshData::ReadJoints(const PMX::Byte*& InOutBufferCursor)
{



}

void PMXMeshData::ReadSoftBodies(const PMX::Byte*& InOutBufferCursor)
{



}
