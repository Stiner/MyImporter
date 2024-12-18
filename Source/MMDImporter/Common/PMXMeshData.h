#pragma once

#include "PMXTypes.h"

/**
 * PMX Mesh Data
 */
class PMXMeshData
{
public:
    bool LoadBinary(const PMX::Byte* const Buffer, const size_t BufferSize);
    void Delete();

protected:
    size_t ReadBuffer(void* OutDest, const PMX::Byte* const InBuffer, const size_t ReadSize);
    size_t ReadText(PMX::Text& OutString, const PMX::Byte* InBuffer);

    bool IsValidPMXFile(const PMX::Header& const Header);

    size_t ReadHeader(const PMX::Byte* const InBuffer);
    size_t ReadModelInfo(const PMX::Byte* const InBuffer);

    size_t ReadVertices(const PMX::Byte* InBuffer);
    size_t ReadSurfaces(const PMX::Byte* InBuffer);
    size_t ReadTextures(const PMX::Byte* InBuffer);
    size_t ReadMaterials(const PMX::Byte* InBuffer);
    size_t ReadBones(const PMX::Byte* InBuffer);
    size_t ReadMorphs(const PMX::Byte* InBuffer);
    size_t ReadDisplayFrames(const PMX::Byte* InBuffer);
    size_t ReadRigidbodies(const PMX::Byte* InBuffer);
    size_t ReadJoints(const PMX::Byte* InBuffer);
    size_t ReadSoftBodies(const PMX::Byte* InBuffer);

protected:
    PMX::Header Header = { 0, };

    PMX::ModelInfo ModelInfo;

    int VertexCount;
    PMX::VertexData* Vertices;

    int SurfaceCount;
    PMX::SurfaceData* Surfaces;

    int TextureCount;
    PMX::TextureData* Textures;

    int MaterialCount;
    PMX::MaterialData* Materials;

    int BoneCount;
    PMX::BoneData* Bones;

    int MorphCount;
    PMX::MorphData* Morphs;

    int DisplayFrameCount;
    PMX::DisplayFrameData* DisplayFrames;

    int RigidbodyCount;
    PMX::RigidbodyData* Rigidbodies;

    int JointCount;
    PMX::JointData* Joints;

    int SoftBodyCount;
    PMX::SoftBodyData* SoftBodies;
};
