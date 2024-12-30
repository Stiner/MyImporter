#pragma once

#include "PMXTypes.h"

namespace PMX
{
    /**
     * PMX Mesh Data
     */
    class PMXMeshData
    {
    public:
        ~PMXMeshData();

        bool LoadBinary(const Byte* const InBuffer, const PMX::MemSize InBufferSize);
        void Delete();

    protected:
        void ReadText(Text* OutString, const Byte*& InOutBufferCursor);

        bool IsValidPMXFile(const Header& Header);

        void ReadHeader(const Byte*& InOutBufferCursor);
        void ReadModelInfo(const Byte*& InOutBufferCursor);
        void ReadVertices(const Byte*& InOutBufferCursor);
        void ReadSurfaces(const Byte*& InOutBufferCursor);
        void ReadTextures(const Byte*& InOutBufferCursor);
        void ReadMaterials(const Byte*& InOutBufferCursor);
        void ReadBones(const Byte*& InOutBufferCursor);
        void ReadMorphs(const Byte*& InOutBufferCursor);
        void ReadDisplayFrames(const Byte*& InOutBufferCursor);
        void ReadRigidbodies(const Byte*& InOutBufferCursor);
        void ReadJoints(const Byte*& InOutBufferCursor);
        void ReadSoftBodies(const Byte*& InOutBufferCursor);

    protected:
        Header HeaderData = { 0, };

        ModelInfo ModelInfoData;

        int VertexCount;
        VertexData* ArrayVertex = nullptr;

        int SurfaceCount;
        SurfaceData* ArraySurface = nullptr;

        int TextureCount;
        TextureData* ArrayTexture = nullptr;

        int MaterialCount;
        MaterialData* ArrayMaterial = nullptr;

        int BoneCount;
        BoneData* ArrayBone = nullptr;

        int MorphCount;
        MorphData* ArrayMorph = nullptr;

        int DisplayFrameCount;
        DisplayFrameData* ArrayDisplayFrame = nullptr;

        int RigidbodyCount;
        RigidbodyData* ArrayRigidbody = nullptr;

        int JointCount;
        JointData* ArrayJoint = nullptr;

        int SoftBodyCount;
        SoftBodyData* ArraySoftBody = nullptr;
    };
}