#pragma once

#include "PMXStructure.h"
#include "PMXText.h"

/**
 * PMX Mesh Data
 */
class PMXMeshData
{
public:
    bool LoadBinary(const PMX::Byte* const Buffer, const PMX::Size_T BufferSize);
    void Delete();

protected:
    static PMX::Size_T ReadBuffer(void* OutDest, const PMX::Byte* const InBuffer, const PMX::Size_T ReadSize);
    static PMX::Size_T ReadText(PMX::Text& OutString, const PMX::Byte* InBuffer, const PMX::EncodingType Encoding);

    PMX::Size_T ReadHeader(const PMX::Byte* const InBuffer);
    PMX::Size_T ReadModelInfo(const PMX::Byte* const InBuffer);
    PMX::Size_T ReadVertex(PMX::Vertex& OutVertex, const PMX::Byte* InBuffer, const PMX::Byte InAdditionalVectorCount, const PMX::Byte InWeightIndexSize);

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
