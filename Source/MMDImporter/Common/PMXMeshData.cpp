#include "PMXMeshData.h"

#include <memory>
#include <cassert>

namespace PMX
{
    void ReadBuffer(void* const OutDest, const Byte*& InOutBufferCursor, const PMX::MemSize ReadSize)
    {
        memcpy(OutDest, InOutBufferCursor, ReadSize);

        InOutBufferCursor += ReadSize;
    }

    void ReadIndex(int* const OutIndex, const Byte*& InOutBufferCursor, const IndexType InIndexType, const Byte InIndexSize)
    {
        assert(InIndexSize == 1 || InIndexSize == 2 || InIndexSize == 4);

        *OutIndex = -1;
        ReadBuffer(OutIndex, InOutBufferCursor, InIndexSize);

        if (InIndexSize == 1)
        {
            if (InIndexType == IndexType::Vertex)
                *OutIndex = static_cast<UInt8>(*OutIndex);
            else
                *OutIndex = static_cast<Int8>(*OutIndex);
        }

        if (InIndexSize == 2)
        {
            if (InIndexType == IndexType::Vertex)
                *OutIndex = static_cast<UInt16>(*OutIndex);
            else
                *OutIndex = static_cast<Int16>(*OutIndex);
        }
    }

    PMXMeshData::~PMXMeshData()
    {
        Delete();
    }

    const Byte* begin = 0;

    bool PMXMeshData::LoadBinary(const Byte* const InBuffer, const PMX::MemSize InBufferSize)
    {
        if (InBuffer == nullptr || InBufferSize == 0)
            return false;

        const Byte* BufferCur = InBuffer;
        begin = BufferCur;

        ReadHeader(BufferCur);

        if (IsValidPMXFile(HeaderData) == false)
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
        ModelInfoData.Delete();

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

    void PMXMeshData::ReadText(Text* OutString, const Byte*& InOutBufferCursor)
    {
        const Text::EncodingType Encoding = HeaderData.TextEncoding;

        OutString->Delete();

        int TextBytesSize = 0;
        ReadBuffer(&TextBytesSize, InOutBufferCursor, sizeof(TextBytesSize));

        if (TextBytesSize == 0)
            return;

        int TextBytesSizeWithNull = 0;
        switch (Encoding)
        {
            case Text::UTF16LE:
                TextBytesSizeWithNull = TextBytesSize + 2;
                break;

            case Text::UTF8:
                TextBytesSizeWithNull = TextBytesSize + 3;
                break;

            default:
                return;
        }

        Byte* TextBuffer = new Byte[TextBytesSizeWithNull]{ 0 };
        ReadBuffer(TextBuffer, InOutBufferCursor, TextBytesSize);

        OutString->SetText(TextBuffer, TextBytesSize, Encoding);
    }

    bool PMXMeshData::IsValidPMXFile(const Header& Header)
    {
        return (Header.Signature[0] == 'P' && Header.Signature[1] == 'M' && Header.Signature[2] == 'X' && Header.Signature[3] == 0x20);
    }

    void PMXMeshData::ReadHeader(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&HeaderData.Signature, InOutBufferCursor, sizeof(HeaderData.Signature));
        ReadBuffer(&HeaderData.Version, InOutBufferCursor, sizeof(HeaderData.Version));

        Int8 GlobalsCount = 0;
        ReadBuffer(&GlobalsCount, InOutBufferCursor, sizeof(GlobalsCount));

        ReadBuffer(&HeaderData.TextEncoding, InOutBufferCursor, GlobalsCount);
    }

    void PMXMeshData::ReadModelInfo(const Byte*& InOutBufferCursor)
    {
        ReadText(&ModelInfoData.NameLocal, InOutBufferCursor);
        ReadText(&ModelInfoData.NameUniversal, InOutBufferCursor);
        ReadText(&ModelInfoData.CommentsLocal, InOutBufferCursor);
        ReadText(&ModelInfoData.CommentsUniversal, InOutBufferCursor);
    }

    void PMXMeshData::ReadVertices(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&VertexCount, InOutBufferCursor, sizeof(VertexCount));

        if (VertexCount <= 0)
            return;

        Vertices = new VertexData[VertexCount];
        memset(Vertices, 0, sizeof(VertexData) * VertexCount);

        for (int i = 0; i < VertexCount; ++i)
        {
            VertexData& Vertex = Vertices[i];

            ReadBuffer(&Vertex.Position, InOutBufferCursor, sizeof(Vertex.Position));
            ReadBuffer(&Vertex.Normal, InOutBufferCursor, sizeof(Vertex.Normal));
            ReadBuffer(&Vertex.UV, InOutBufferCursor, sizeof(Vertex.UV));

            ReadBuffer(&Vertex.Additional, InOutBufferCursor, sizeof(Vertex.Additional[0]) * HeaderData.AdditionalVectorCount);

            ReadBuffer(&Vertex.WeightDeformType, InOutBufferCursor, sizeof(Vertex.WeightDeformType));

            switch (Vertex.WeightDeformType)
            {
                case VertexData::WeightDeformType::BDEF1:
                    {
                        auto& BDef1 = Vertex.WeightDeform.BDef1;

                        ReadIndex(&BDef1.BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                    }
                    break;
                case VertexData::WeightDeformType::BDEF2:
                    {
                        auto& BDef2 = Vertex.WeightDeform.BDef2;

                        ReadIndex(&BDef2.BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef2.BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&BDef2.Weight0, InOutBufferCursor, sizeof(BDef2.Weight0));
                        BDef2.Weight1 = 1.0f - BDef2.Weight0;
                    }
                    break;
                case VertexData::WeightDeformType::BDEF4:
                    {
                        auto& BDef4 = Vertex.WeightDeform.BDef4;

                        ReadIndex(&BDef4.BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef4.BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef4.BoneIndex2, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef4.BoneIndex3, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&BDef4.Weight0, InOutBufferCursor, sizeof(BDef4.Weight0));
                        ReadBuffer(&BDef4.Weight1, InOutBufferCursor, sizeof(BDef4.Weight1));
                        ReadBuffer(&BDef4.Weight2, InOutBufferCursor, sizeof(BDef4.Weight2));
                        ReadBuffer(&BDef4.Weight3, InOutBufferCursor, sizeof(BDef4.Weight3));
                    }
                    break;
                case VertexData::WeightDeformType::SDEF:
                    {
                        auto& SDef = Vertex.WeightDeform.SDef;

                        ReadIndex(&SDef.BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&SDef.BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&SDef.Weight0, InOutBufferCursor, sizeof(SDef.Weight0));
                        SDef.Weight1 = 1.0f - SDef.Weight0;

                        ReadBuffer(&SDef.C, InOutBufferCursor, sizeof(SDef.C));
                        ReadBuffer(&SDef.R0, InOutBufferCursor, sizeof(SDef.R0));
                        ReadBuffer(&SDef.R1, InOutBufferCursor, sizeof(SDef.R1));
                    }
                    break;
                case VertexData::WeightDeformType::QDEF:
                    {
                        auto& QDef = Vertex.WeightDeform.QDef;

                        ReadIndex(&QDef.BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&QDef.BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&QDef.BoneIndex2, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&QDef.BoneIndex3, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&QDef.Weight0, InOutBufferCursor, sizeof(QDef.Weight0));
                        ReadBuffer(&QDef.Weight1, InOutBufferCursor, sizeof(QDef.Weight1));
                        ReadBuffer(&QDef.Weight2, InOutBufferCursor, sizeof(QDef.Weight2));
                        ReadBuffer(&QDef.Weight3, InOutBufferCursor, sizeof(QDef.Weight3));
                    }
                    break;
            }

            ReadBuffer(&Vertex.EdgeScale, InOutBufferCursor, sizeof(Vertex.EdgeScale));
        }
    }

    void PMXMeshData::ReadSurfaces(const Byte*& InOutBufferCursor)
    {
        int IndexCount = 0;
        ReadBuffer(&IndexCount, InOutBufferCursor, sizeof(IndexCount));

        if (IndexCount <= 0)
            return;

        // 3개로 하나의 삼각형 구성
        SurfaceCount = IndexCount / 3;

        Surfaces = new SurfaceData[SurfaceCount];
        memset(Surfaces, 0, sizeof(SurfaceData) * SurfaceCount);

        for (int i = 0; i < SurfaceCount; ++i)
        {
            ReadIndex(&Surfaces[i].VertexIndex[0], InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
            ReadIndex(&Surfaces[i].VertexIndex[1], InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
            ReadIndex(&Surfaces[i].VertexIndex[2], InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
        }
    }

    void PMXMeshData::ReadTextures(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&TextureCount, InOutBufferCursor, sizeof(TextureCount));

        if (TextureCount <= 0)
            return;

        Textures = new TextureData[TextureCount];
        memset(Textures, 0, sizeof(TextureData) * TextureCount);

        for (int i = 0; i < TextureCount; ++i)
        {
            ReadText(&Textures[i].Path, InOutBufferCursor);
        }
    }

    void PMXMeshData::ReadMaterials(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&MaterialCount, InOutBufferCursor, sizeof(MaterialCount));

        if (MaterialCount <= 0)
            return;

        Materials = new MaterialData[MaterialCount];
        memset(Materials, 0, sizeof(MaterialData) * MaterialCount);

        for (int i = 0; i < MaterialCount; ++i)
        {
            MaterialData& MaterialData = Materials[i];

            ReadText(&MaterialData.NameLocal, InOutBufferCursor);
            ReadText(&MaterialData.NameUniversal, InOutBufferCursor);
            ReadBuffer(&MaterialData.DiffuseColor, InOutBufferCursor, sizeof(MaterialData.DiffuseColor));
            ReadBuffer(&MaterialData.SpecularColor, InOutBufferCursor, sizeof(MaterialData.SpecularColor));
            ReadBuffer(&MaterialData.SpecularStrength, InOutBufferCursor, sizeof(MaterialData.SpecularStrength));
            ReadBuffer(&MaterialData.AmbientColor, InOutBufferCursor, sizeof(MaterialData.AmbientColor));
            ReadBuffer(&MaterialData.DrawingFlags, InOutBufferCursor, sizeof(MaterialData.DrawingFlags));
            ReadBuffer(&MaterialData.EdgeColor, InOutBufferCursor, sizeof(MaterialData.EdgeColor));
            ReadBuffer(&MaterialData.EdgeScale, InOutBufferCursor, sizeof(MaterialData.EdgeScale));
            ReadIndex(&MaterialData.TextureIndex, InOutBufferCursor, IndexType::Texture, HeaderData.TextureIndexSize);
            ReadIndex(&MaterialData.EnvironmentTextureIndex, InOutBufferCursor, IndexType::Texture, HeaderData.TextureIndexSize);
            ReadBuffer(&MaterialData.EnvironmentBlendMode, InOutBufferCursor, sizeof(MaterialData.EnvironmentBlendMode));
            ReadBuffer(&MaterialData.ToonReference, InOutBufferCursor, sizeof(MaterialData.ToonReference));
            ReadBuffer(&MaterialData.ToonValue, InOutBufferCursor, sizeof(MaterialData.ToonValue));
            ReadText(&MaterialData.MetaData, InOutBufferCursor);
            ReadBuffer(&MaterialData.SurfaceCount, InOutBufferCursor, sizeof(MaterialData.SurfaceCount));
        }
    }

    void PMXMeshData::ReadBones(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&BoneCount, InOutBufferCursor, sizeof(BoneCount));

        if (BoneCount <= 0)
            return;

        Bones = new BoneData[BoneCount];
        memset(Bones, 0, sizeof(BoneData) * BoneCount);

        for (int i = 0; i < BoneCount; ++i)
        {
            BoneData& BoneData = Bones[i];

            ReadText(&BoneData.NameLocal, InOutBufferCursor);
            ReadText(&BoneData.NameUniversal, InOutBufferCursor);
            ReadBuffer(&BoneData.Position, InOutBufferCursor, sizeof(BoneData.Position));
            ReadIndex(&BoneData.ParentBoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
            ReadBuffer(&BoneData.Layer, InOutBufferCursor, sizeof(BoneData.Layer));
            ReadBuffer(&BoneData.Flags, InOutBufferCursor, sizeof(BoneData.Flags));

            if (BoneData.Flags & BoneData::IndexedTailPosition)
            {
                ReadIndex(&BoneData.TailPosition.BoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
            }
            else
            {
                ReadBuffer(&BoneData.TailPosition.Vector3, InOutBufferCursor, sizeof(BoneData.TailPosition.Vector3));
            }

            if (BoneData.Flags & (BoneData::InheritRotation | BoneData::InheritTranslation))
            {
                ReadIndex(&BoneData.InheritBoneData.ParentBoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                ReadBuffer(&BoneData.InheritBoneData.ParentInfluence, InOutBufferCursor, sizeof(BoneData.InheritBoneData.ParentInfluence));
            }

            if (BoneData.Flags & (BoneData::FixedAxis))
            {
                ReadBuffer(&BoneData.FixedAxisData, InOutBufferCursor, sizeof(BoneData.FixedAxisData));
            }

            if (BoneData.Flags & (BoneData::LocalCoordinate))
            {
                ReadBuffer(&BoneData.LocalCoordinateData, InOutBufferCursor, sizeof(BoneData.LocalCoordinateData));
            }

            if (BoneData.Flags & (BoneData::ExternalParentDeform))
            {
                ReadIndex(&BoneData.ExternalParentData.ParentBoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
            }

            if (BoneData.Flags & (BoneData::IK))
            {
                auto& IKData = BoneData.IKData;

                ReadIndex(&IKData.TargetIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                ReadBuffer(&IKData.LoopCount, InOutBufferCursor, sizeof(IKData.LoopCount));
                ReadBuffer(&IKData.LimitRadian, InOutBufferCursor, sizeof(IKData.LimitRadian));
                ReadBuffer(&IKData.LinkCount, InOutBufferCursor, sizeof(IKData.LinkCount));

                if (IKData.LinkCount > 0)
                {
                    IKData.Links = new BoneData::BoneIK::IKLink[IKData.LinkCount];
                    memset(IKData.Links, 0, sizeof(BoneData::BoneIK::IKLink) * IKData.LinkCount);

                    for (int j = 0, max = IKData.LinkCount; j < max; ++j)
                    {
                        auto& LinkData = IKData.Links[j];

                        ReadIndex(&LinkData.BoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadBuffer(&LinkData.HasLimit, InOutBufferCursor, sizeof(LinkData.HasLimit));

                        if (LinkData.HasLimit != 0)
                        {
                            ReadBuffer(&LinkData.Limit, InOutBufferCursor, sizeof(LinkData.Limit));
                        }
                    }
                }
            }
        }
    }

    void PMXMeshData::ReadMorphs(const Byte*& InOutBufferCursor)
    {
    }

    void PMXMeshData::ReadDisplayFrames(const Byte*& InOutBufferCursor)
    {
    }

    void PMXMeshData::ReadRigidbodies(const Byte*& InOutBufferCursor)
    {
    }

    void PMXMeshData::ReadJoints(const Byte*& InOutBufferCursor)
    {
    }

    void PMXMeshData::ReadSoftBodies(const Byte*& InOutBufferCursor)
    {
    }
}