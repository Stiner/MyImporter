#include "PMXMeshData.h"

#include <memory>
#include <cassert>

bool PMXMeshData::LoadBinary(const PMX::Byte* const InBuffer, const PMX::Size_T InBufferSize)
{
    if (InBuffer == nullptr || InBufferSize == 0)
        return false;

    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadHeader(BufferCur);

    if ((Header.Signature[0] == 'P' && Header.Signature[1] == 'M' && Header.Signature[2] == 'X') == false)
        return false;

    BufferCur += ReadModelInfo(BufferCur);

    BufferCur += ReadBuffer(&VertexCount, BufferCur, sizeof(VertexCount));
    Vertices = new PMX::Vertex[VertexCount];
    for (int i = 0; i < VertexCount; ++i)
    {
        BufferCur += ReadVertex(Vertices[i], BufferCur, Header.AdditionalVectorCount, Header.VertexIndexSize);
    }

    // TODO : 나머지 것들 로드


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

    if (Vertices != nullptr)
        delete Vertices;
    Vertices = nullptr;

    VertexCount = 0;

    SurfaceCount = 0;

    TextureCount = 0;

    MaterialCount = 0;

    BoneCount = 0;

    MorphCount = 0;

    DisplayframeCount = 0;

    RigidbodyCount = 0;

    JointCount = 0;

    SoftBodyCount = 0;

}

PMX::Size_T PMXMeshData::ReadBuffer(void* OutDest, const PMX::Byte* const InBuffer, const PMX::Size_T ReadSize)
{
    memcpy(OutDest, InBuffer, ReadSize);
    return ReadSize;
}

PMX::Size_T PMXMeshData::ReadText(PMX::Text& OutString, const PMX::Byte* InBuffer, const PMX::EncodingType InEncoding)
{
    const PMX::Byte* BufferCur = InBuffer;

    OutString.Delete();

    int TextBytesSize = 0;
    int TextBytesSizeWithNull = 0;
    BufferCur += ReadBuffer(&TextBytesSize, BufferCur, sizeof(TextBytesSize));

    switch (InEncoding)
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

    OutString.FromBytes(TextBuffer, TextBytesSize, InEncoding);

    return BufferCur - InBuffer;
}

PMX::Size_T PMXMeshData::ReadHeader(const PMX::Byte* const InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadBuffer(Header.Signature, BufferCur, sizeof(Header.Signature));
    BufferCur += ReadBuffer(&Header.Version, BufferCur, sizeof(Header.Version));

    PMX::Byte GlobalsCount = 0;
    BufferCur += ReadBuffer(&GlobalsCount, BufferCur, sizeof(PMX::Byte));

    BufferCur += ReadBuffer(&Header.TextEncoding, BufferCur, sizeof(PMX::Byte) * GlobalsCount);

    return BufferCur - InBuffer;
}

PMX::Size_T PMXMeshData::ReadModelInfo(const PMX::Byte* const InBuffer)
{
    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadText(ModelInfo.ModelNameLocal, BufferCur, Header.TextEncoding);
    BufferCur += ReadText(ModelInfo.ModelNameUniversal, BufferCur, Header.TextEncoding);
    BufferCur += ReadText(ModelInfo.CommentsLocal, BufferCur, Header.TextEncoding);
    BufferCur += ReadText(ModelInfo.CommentsUniversal, BufferCur, Header.TextEncoding);

    return BufferCur - InBuffer;
}

PMX::Size_T PMXMeshData::ReadVertex(PMX::Vertex& OutVertex, const PMX::Byte* InBuffer, const PMX::Byte InAdditionalVectorCount, const PMX::Byte InWeightIndexSize)
{
    memset(&OutVertex, 0, sizeof(PMX::Vertex));

    const PMX::Byte* BufferCur = InBuffer;

    BufferCur += ReadBuffer(&OutVertex.Position, BufferCur, sizeof(OutVertex.Position));
    BufferCur += ReadBuffer(&OutVertex.Normal, BufferCur, sizeof(OutVertex.Normal));
    BufferCur += ReadBuffer(&OutVertex.UV, BufferCur, sizeof(OutVertex.UV));

    BufferCur += ReadBuffer(OutVertex.Additional, BufferCur, sizeof(OutVertex.Additional[0]) * InAdditionalVectorCount);

    BufferCur += ReadBuffer(&OutVertex.WeightDeformType, BufferCur, sizeof(OutVertex.WeightDeformType));
    {
        switch (OutVertex.WeightDeformType)
        {
            case PMX::WeightDeformType::BDEF1:
                {
                    auto& WeightDeform = OutVertex.WeightDeform.BDef1;

                    BufferCur += ReadBuffer(&WeightDeform.Index, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                }
                break;
            case PMX::WeightDeformType::BDEF2:
                {
                    auto& WeightDeform = OutVertex.WeightDeform.BDef2;

                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index2, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Weight1, BufferCur, sizeof(WeightDeform.Weight1));
                    WeightDeform.Weight2 = 1.0f - WeightDeform.Weight1;
                }
                break;
            case PMX::WeightDeformType::BDEF4:
                {
                    auto& WeightDeform = OutVertex.WeightDeform.BDef4;

                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index2, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index3, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index4, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Weight1, BufferCur, sizeof(WeightDeform.Weight1));
                    BufferCur += ReadBuffer(&WeightDeform.Weight2, BufferCur, sizeof(WeightDeform.Weight2));
                    BufferCur += ReadBuffer(&WeightDeform.Weight3, BufferCur, sizeof(WeightDeform.Weight3));
                    BufferCur += ReadBuffer(&WeightDeform.Weight4, BufferCur, sizeof(WeightDeform.Weight4));
                }
                break;
            case PMX::WeightDeformType::SDEF:
                {
                    auto& WeightDeform = OutVertex.WeightDeform.SDef;

                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index2, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Weight1, BufferCur, sizeof(WeightDeform.Weight1));
                    WeightDeform.Weight2 = 1.0f - WeightDeform.Weight1;
                    BufferCur += ReadBuffer(&WeightDeform.C, BufferCur, sizeof(WeightDeform.C));
                    BufferCur += ReadBuffer(&WeightDeform.R0, BufferCur, sizeof(WeightDeform.R0));
                    BufferCur += ReadBuffer(&WeightDeform.R1, BufferCur, sizeof(WeightDeform.R1));
                }
                break;
            case PMX::WeightDeformType::QDEF:
                {
                    auto& WeightDeform = OutVertex.WeightDeform.QDef;

                    BufferCur += ReadBuffer(&WeightDeform.Index1, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index2, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index3, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Index4, BufferCur, sizeof(PMX::Byte) * InWeightIndexSize);
                    BufferCur += ReadBuffer(&WeightDeform.Weight1, BufferCur, sizeof(WeightDeform.Weight1));
                    BufferCur += ReadBuffer(&WeightDeform.Weight2, BufferCur, sizeof(WeightDeform.Weight2));
                    BufferCur += ReadBuffer(&WeightDeform.Weight3, BufferCur, sizeof(WeightDeform.Weight3));
                    BufferCur += ReadBuffer(&WeightDeform.Weight4, BufferCur, sizeof(WeightDeform.Weight4));
                }
                break;

            default:
                assert(0);
                break;
        }
    }

    BufferCur += ReadBuffer(&OutVertex.EdgeScale, BufferCur, sizeof(OutVertex.EdgeScale));

    return BufferCur - InBuffer;
}

