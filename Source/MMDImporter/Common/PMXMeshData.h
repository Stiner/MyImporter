#pragma once

#include "PMXStructure.h"
#include "PMXText.h"

/**
 * PMX Mesh Data
 */
class PMXMeshData
{
public:
    bool LoadBinary(const PMX::Byte* const Buffer, const size_t BufferSize);
    void Delete();

protected:
    static size_t ReadBuffer(void* OutDest, const PMX::Byte* const InBuffer, const size_t ReadSize);
    static size_t ReadText(PMX::Text& OutString, const PMX::Byte* InBuffer, const PMX::EncodingType Encoding);

    size_t ReadHeader(const PMX::Byte* const InBuffer);
    size_t ReadModelInfo(const PMX::Byte* const InBuffer);
    size_t ReadVertex(PMX::Vertex& OutVertex, const PMX::Byte* InBuffer, const PMX::Byte InAdditionalVectorCount, const PMX::Byte InWeightIndexSize);

protected:
    PMX::Header Header = { 0, };

    PMX::ModelInfo ModelInfo;

    int VertexCount;
    PMX::Vertex* Vertices;

    int SurfaceCount;

    int TextureCount;

    int MaterialCount;

    int BoneCount;

    int MorphCount;

    int DisplayframeCount;

    int RigidbodyCount;

    int JointCount;

    int SoftBodyCount;
};
