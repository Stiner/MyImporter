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

        int VertexCount = 0;
        VertexData* ArrayVertex = nullptr;

        int SurfaceCount = 0;
        SurfaceData* ArraySurface = nullptr;

        int TextureCount = 0;
        TextureData* ArrayTexture = nullptr;

        int MaterialCount = 0;
        MaterialData* ArrayMaterial = nullptr;

        int BoneCount = 0;
        BoneData* ArrayBone = nullptr;

        int MorphCount = 0;
        MorphData* ArrayMorph = nullptr;

        int DisplayFrameCount = 0;
        DisplayFrameData* ArrayDisplayFrame = nullptr;

        int RigidbodyCount = 0;
        RigidbodyData* ArrayRigidbody = nullptr;

        int JointCount = 0;
        JointData* ArrayJoint = nullptr;

        int SoftBodyCount = 0;
        SoftBodyData* ArraySoftBody = nullptr;
    };
}