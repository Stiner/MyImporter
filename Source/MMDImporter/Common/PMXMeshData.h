#pragma once

#include "PMXTypes.h"

/**
 * PMX Mesh Data
 */
class PMXMeshData
{
public:
    ~PMXMeshData();

    bool LoadBinary(const PMX::Byte* const Buffer, const size_t BufferSize);
    void Delete();

protected:
    void ReadText(PMX::Text& OutString, const PMX::Byte*& InOutBufferCursor);

    bool IsValidPMXFile(const PMX::Header& Header);

    void ReadHeader(const PMX::Byte*& InOutBufferCursor);
    void ReadModelInfo(const PMX::Byte*& InOutBufferCursor);
    void ReadVertices(const PMX::Byte*& InOutBufferCursor);
    void ReadSurfaces(const PMX::Byte*& InOutBufferCursor);
    void ReadTextures(const PMX::Byte*& InOutBufferCursor);
    void ReadMaterials(const PMX::Byte*& InOutBufferCursor);
    void ReadBones(const PMX::Byte*& InOutBufferCursor);
    void ReadMorphs(const PMX::Byte*& InOutBufferCursor);
    void ReadDisplayFrames(const PMX::Byte*& InOutBufferCursor);
    void ReadRigidbodies(const PMX::Byte*& InOutBufferCursor);
    void ReadJoints(const PMX::Byte*& InOutBufferCursor);
    void ReadSoftBodies(const PMX::Byte*& InOutBufferCursor);

protected:
    PMX::Header Header = { 0, };

    PMX::ModelInfo ModelInfo;

    int VertexCount;
    PMX::VertexData* Vertices = nullptr;

    int SurfaceCount;
    PMX::SurfaceData* Surfaces = nullptr;

    int TextureCount;
    PMX::TextureData* Textures = nullptr;

    int MaterialCount;
    PMX::MaterialData* Materials = nullptr;

    int BoneCount;
    PMX::BoneData* Bones = nullptr;

    int MorphCount;
    PMX::MorphData* Morphs = nullptr;

    int DisplayFrameCount;
    PMX::DisplayFrameData* DisplayFrames = nullptr;

    int RigidbodyCount;
    PMX::RigidbodyData* Rigidbodies = nullptr;

    int JointCount;
    PMX::JointData* Joints = nullptr;

    int SoftBodyCount;
    PMX::SoftBodyData* SoftBodies = nullptr;
};
