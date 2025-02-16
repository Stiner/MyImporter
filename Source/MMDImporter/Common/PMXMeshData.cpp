﻿#include "PMXMeshData.h"

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

    template <class T>
    T* AllocDataArray(int Size)
    {
        if (Size <= 0)
            return nullptr;

        T* Array = new T[Size];
        return reinterpret_cast<T*>(memset(Array, 0, sizeof(T) * Size));
    }

    PMXMeshData::~PMXMeshData()
    {
        Delete();
    }

    bool PMXMeshData::LoadBinary(const Byte* const InBuffer, const PMX::MemSize InBufferSize)
    {
        if (InBuffer == nullptr || InBufferSize == 0)
            return false;

        const Byte* BufferCur = InBuffer;
        const Byte* BufferEnd = InBuffer + InBufferSize;

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

        if (HeaderData.Version > 2.0f)
        {
            ReadSoftBodies(BufferCur);
        }

        // 끝까지 정상적으로 읽었는지 검사
        if (BufferCur != BufferEnd)
        {
            Delete();
            return false;
        }

        return true;
    }

    void PMXMeshData::Delete()
    {
        ModelInfoData.Delete();

        PMX_SAFE_DELETE_ARRAY(ArrayVertex);
        VertexCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArraySurface);
        SurfaceCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayTexture);
        TextureCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayMaterial);
        MaterialCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayBone);
        BoneCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayMorph);
        MorphCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayDisplayFrame);
        DisplayFrameCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayRigidbody);
        RigidbodyCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArrayJoint);
        JointCount = 0;

        PMX_SAFE_DELETE_ARRAY(ArraySoftBody);
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

        ArrayVertex = AllocDataArray<VertexData>(VertexCount);

        for (int i = 0; i < VertexCount; ++i)
        {
            VertexData& Vertex = ArrayVertex[i];

            ReadBuffer(&Vertex.Position, InOutBufferCursor, sizeof(Vertex.Position));
            ReadBuffer(&Vertex.Normal, InOutBufferCursor, sizeof(Vertex.Normal));
            ReadBuffer(&Vertex.UV, InOutBufferCursor, sizeof(Vertex.UV));

            ReadBuffer(&Vertex.Additional, InOutBufferCursor, sizeof(Vertex.Additional[0]) * HeaderData.AdditionalVectorCount);

            ReadBuffer(&Vertex.DeformType, InOutBufferCursor, sizeof(Vertex.DeformType));

            switch (Vertex.DeformType)
            {
                case VertexData::WeightDeformType::BDEF1:
                    {
                        Vertex.Deform = new VertexData::BDEF1();
                        VertexData::BDEF1* BDef1 = (VertexData::BDEF1*)Vertex.Deform;

                        ReadIndex(&BDef1->BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                    }
                    break;
                case VertexData::WeightDeformType::BDEF2:
                    {
                        Vertex.Deform = new VertexData::BDEF2();
                        VertexData::BDEF2* BDef2 = (VertexData::BDEF2*)Vertex.Deform;

                        ReadIndex(&BDef2->BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef2->BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&BDef2->Weight0, InOutBufferCursor, sizeof(BDef2->Weight0));
                        BDef2->Weight1 = 1.0f - BDef2->Weight0;
                    }
                    break;
                case VertexData::WeightDeformType::BDEF4:
                    {
                        Vertex.Deform = new VertexData::BDEF4();
                        VertexData::BDEF4* BDef4 = (VertexData::BDEF4*)Vertex.Deform;

                        ReadIndex(&BDef4->BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef4->BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef4->BoneIndex2, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&BDef4->BoneIndex3, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&BDef4->Weight0, InOutBufferCursor, sizeof(BDef4->Weight0));
                        ReadBuffer(&BDef4->Weight1, InOutBufferCursor, sizeof(BDef4->Weight1));
                        ReadBuffer(&BDef4->Weight2, InOutBufferCursor, sizeof(BDef4->Weight2));
                        ReadBuffer(&BDef4->Weight3, InOutBufferCursor, sizeof(BDef4->Weight3));
                    }
                    break;
                case VertexData::WeightDeformType::SDEF:
                    {
                        Vertex.Deform = new VertexData::SDEF();
                        VertexData::SDEF* SDef = (VertexData::SDEF*)Vertex.Deform;

                        ReadIndex(&SDef->BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&SDef->BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&SDef->Weight0, InOutBufferCursor, sizeof(SDef->Weight0));
                        SDef->Weight1 = 1.0f - SDef->Weight0;

                        ReadBuffer(&SDef->C, InOutBufferCursor, sizeof(SDef->C));
                        ReadBuffer(&SDef->R0, InOutBufferCursor, sizeof(SDef->R0));
                        ReadBuffer(&SDef->R1, InOutBufferCursor, sizeof(SDef->R1));
                    }
                    break;
                case VertexData::WeightDeformType::QDEF:
                    {
                        Vertex.Deform = new VertexData::QDEF();
                        VertexData::QDEF* QDef = (VertexData::QDEF*)Vertex.Deform;

                        ReadIndex(&QDef->BoneIndex0, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&QDef->BoneIndex1, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&QDef->BoneIndex2, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadIndex(&QDef->BoneIndex3, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

                        ReadBuffer(&QDef->Weight0, InOutBufferCursor, sizeof(QDef->Weight0));
                        ReadBuffer(&QDef->Weight1, InOutBufferCursor, sizeof(QDef->Weight1));
                        ReadBuffer(&QDef->Weight2, InOutBufferCursor, sizeof(QDef->Weight2));
                        ReadBuffer(&QDef->Weight3, InOutBufferCursor, sizeof(QDef->Weight3));
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

        ArraySurface = AllocDataArray<SurfaceData>(SurfaceCount);

        for (int i = 0; i < SurfaceCount; ++i)
        {
            ReadIndex(&ArraySurface[i].VertexIndex[0], InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
            ReadIndex(&ArraySurface[i].VertexIndex[1], InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
            ReadIndex(&ArraySurface[i].VertexIndex[2], InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
        }
    }

    void PMXMeshData::ReadTextures(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&TextureCount, InOutBufferCursor, sizeof(TextureCount));

        if (TextureCount <= 0)
            return;

        ArrayTexture = AllocDataArray<TextureData>(TextureCount);

        for (int i = 0; i < TextureCount; ++i)
        {
            ReadText(&ArrayTexture[i].Path, InOutBufferCursor);
        }
    }

    void PMXMeshData::ReadMaterials(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&MaterialCount, InOutBufferCursor, sizeof(MaterialCount));

        if (MaterialCount <= 0)
            return;

        ArrayMaterial = AllocDataArray<MaterialData>(MaterialCount);

        for (int i = 0; i < MaterialCount; ++i)
        {
            MaterialData& MaterialData = ArrayMaterial[i];

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

        ArrayBone = AllocDataArray<BoneData>(BoneCount);

        for (int i = 0; i < BoneCount; ++i)
        {
            BoneData& BoneData = ArrayBone[i];

            ReadText(&BoneData.NameLocal, InOutBufferCursor);
            ReadText(&BoneData.NameUniversal, InOutBufferCursor);
            ReadBuffer(&BoneData.Position, InOutBufferCursor, sizeof(BoneData.Position));
            ReadIndex(&BoneData.ParentBoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
            ReadBuffer(&BoneData.Layer, InOutBufferCursor, sizeof(BoneData.Layer));
            ReadBuffer(&BoneData.Flags, InOutBufferCursor, sizeof(BoneData.Flags));

            if (BoneData.Flags & (BoneData::Flag::IndexedTailPosition))
            {
                ReadIndex(&BoneData.TailPositionData.BoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
            }
            else
            {
                ReadBuffer(&BoneData.TailPositionData.Vector3, InOutBufferCursor, sizeof(BoneData.TailPositionData.Vector3));
            }

            if (BoneData.Flags & (BoneData::Flag::InheritRotation | BoneData::Flag::InheritTranslation))
            {
                BoneData.InheritBoneData = new struct BoneData::InheritBone();

                ReadIndex(&BoneData.InheritBoneData->ParentBoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                ReadBuffer(&BoneData.InheritBoneData->ParentInfluence, InOutBufferCursor, sizeof(BoneData.InheritBoneData->ParentInfluence));
            }

            if (BoneData.Flags & (BoneData::Flag::FixedAxis))
            {
                BoneData.FixedAxisData = new struct BoneData::FixedAxis();

                ReadBuffer(&BoneData.FixedAxisData->AxisDirection, InOutBufferCursor, sizeof(BoneData.FixedAxisData->AxisDirection));
            }

            if (BoneData.Flags & (BoneData::Flag::LocalCoordinate))
            {
                BoneData.LocalCoordinateData = new struct BoneData::LocalCoordinate();

                ReadBuffer(&BoneData.LocalCoordinateData->XVector, InOutBufferCursor, sizeof(BoneData.LocalCoordinateData->XVector));
                ReadBuffer(&BoneData.LocalCoordinateData->ZVector, InOutBufferCursor, sizeof(BoneData.LocalCoordinateData->ZVector));
            }

            if (BoneData.Flags & (BoneData::Flag::ExternalParentDeform))
            {
                BoneData.ExternalParentData = new struct BoneData::ExternalParent();

                ReadIndex(&BoneData.ExternalParentData->ParentBoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
            }

            if (BoneData.Flags & (BoneData::Flag::UseIK))
            {
                ReadIndex(&BoneData.IKData.TargetIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                ReadBuffer(&BoneData.IKData.LoopCount, InOutBufferCursor, sizeof(BoneData.IKData.LoopCount));
                ReadBuffer(&BoneData.IKData.LimitRadian, InOutBufferCursor, sizeof(BoneData.IKData.LimitRadian));
                ReadBuffer(&BoneData.IKData.LinkCount, InOutBufferCursor, sizeof(BoneData.IKData.LinkCount));

                if (BoneData.IKData.LinkCount > 0)
                {
                    BoneData.IKData.ArrayLink = new BoneData::IK::LinkData[BoneData.IKData.LinkCount];
                    memset(BoneData.IKData.ArrayLink, 0, sizeof(BoneData::IK::LinkData) * BoneData.IKData.LinkCount);

                    for (int j = 0, max = BoneData.IKData.LinkCount; j < max; ++j)
                    {
                        auto& LinkData = BoneData.IKData.ArrayLink[j];

                        ReadIndex(&LinkData.BoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                        ReadBuffer(&LinkData.HasLimit, InOutBufferCursor, sizeof(LinkData.HasLimit));

                        if (LinkData.HasLimit != 0)
                        {
                            ReadBuffer(&LinkData.LimitData.Min, InOutBufferCursor, sizeof(LinkData.LimitData.Min));
                            ReadBuffer(&LinkData.LimitData.Max, InOutBufferCursor, sizeof(LinkData.LimitData.Max));
                        }
                    }
                }
            }
        }
    }

    void PMXMeshData::ReadMorphs(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&MorphCount, InOutBufferCursor, sizeof(MorphCount));

        if (MorphCount <= 0)
            return;

        ArrayMorph = AllocDataArray<MorphData>(MorphCount);

        for (int i = 0; i < MorphCount; ++i)
        {
            MorphData& MorphData = ArrayMorph[i];

            ReadText(&MorphData.NameLocal, InOutBufferCursor);
            ReadText(&MorphData.NameUniversal, InOutBufferCursor);
            ReadBuffer(&MorphData.PanelType, InOutBufferCursor, sizeof(MorphData.PanelType));
            ReadBuffer(&MorphData.Type, InOutBufferCursor, sizeof(MorphData.Type));
            ReadBuffer(&MorphData.OffsetCount, InOutBufferCursor, sizeof(MorphData.OffsetCount));

            if (MorphData.OffsetCount > 0)
            {
                MorphData::OffsetBase*& Offsets = MorphData.ArrayOffset;

                switch (MorphData.Type)
                {
                    case MorphData::MorphType::Group:
                        {
                            Offsets = new MorphData::OffsetGroup[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetGroup) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                MorphData::OffsetGroup& OffsetData = ((MorphData::OffsetGroup*)Offsets)[j];

                                ReadIndex(&OffsetData.MorphIndex, InOutBufferCursor, IndexType::Morph, HeaderData.MorphIndexSize);
                                ReadBuffer(&OffsetData.Rate, InOutBufferCursor, sizeof(OffsetData.Rate));
                            }
                        }
                        break;
                    case MorphData::MorphType::Vertex:
                        {
                            Offsets = new MorphData::OffsetVertex[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetVertex) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                MorphData::OffsetVertex& OffsetData = ((MorphData::OffsetVertex*)Offsets)[j];

                                ReadIndex(&OffsetData.VertexIndex, InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
                                ReadBuffer(&OffsetData.PositionOffset, InOutBufferCursor, sizeof(OffsetData.PositionOffset));
                            }
                        }
                        break;
                    case MorphData::MorphType::Bone:
                        {
                            Offsets = new MorphData::OffsetBone[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetBone) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                auto& OffsetData = ((MorphData::OffsetBone*)Offsets)[j];

                                ReadIndex(&OffsetData.BoneIndex, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                                ReadBuffer(&OffsetData.MoveValue, InOutBufferCursor, sizeof(OffsetData.MoveValue));
                                ReadBuffer(&OffsetData.RotationValue, InOutBufferCursor, sizeof(OffsetData.RotationValue));
                            }
                        }
                        break;
                    case MorphData::MorphType::UV:
                    case MorphData::MorphType::AdditionalUV1:
                    case MorphData::MorphType::AdditionalUV2:
                    case MorphData::MorphType::AdditionalUV3:
                    case MorphData::MorphType::AdditionalUV4:
                        {
                            Offsets = new MorphData::OffsetUV[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetUV) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                auto& OffsetData = ((MorphData::OffsetUV*)Offsets)[j];

                                ReadIndex(&OffsetData.VertexIndex, InOutBufferCursor, IndexType::Vertex, HeaderData.VertexIndexSize);
                                ReadBuffer(&OffsetData.UVOffset, InOutBufferCursor, sizeof(OffsetData.UVOffset));
                            }
                        }
                        break;
                    case MorphData::MorphType::Material:
                        {
                            Offsets = new MorphData::OffsetMaterial[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetMaterial) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                auto& OffsetData = ((MorphData::OffsetMaterial*)Offsets)[j];

                                ReadIndex(&OffsetData.MaterialIndex, InOutBufferCursor, IndexType::Material, HeaderData.MaterialIndexSize);
                                ReadBuffer(&OffsetData.OffsetMethod, InOutBufferCursor, sizeof(OffsetData.OffsetMethod));
                                ReadBuffer(&OffsetData.DiffuseColor, InOutBufferCursor, sizeof(OffsetData.DiffuseColor));
                                ReadBuffer(&OffsetData.SpecularColor, InOutBufferCursor, sizeof(OffsetData.SpecularColor));
                                ReadBuffer(&OffsetData.Specularity, InOutBufferCursor, sizeof(OffsetData.Specularity));
                                ReadBuffer(&OffsetData.AmbientColor, InOutBufferCursor, sizeof(OffsetData.AmbientColor));
                                ReadBuffer(&OffsetData.EdgeColor, InOutBufferCursor, sizeof(OffsetData.EdgeColor));
                                ReadBuffer(&OffsetData.EdgeSize, InOutBufferCursor, sizeof(OffsetData.EdgeSize));
                                ReadBuffer(&OffsetData.TextureTint, InOutBufferCursor, sizeof(OffsetData.TextureTint));
                                ReadBuffer(&OffsetData.EnvironmentTint, InOutBufferCursor, sizeof(OffsetData.EnvironmentTint));
                                ReadBuffer(&OffsetData.ToonTint, InOutBufferCursor, sizeof(OffsetData.ToonTint));
                            }
                        }
                        break;
                    case MorphData::MorphType::Flip:
                        {
                            Offsets = new MorphData::OffsetFlip[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetFlip) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                auto& OffsetData = ((MorphData::OffsetFlip*)Offsets)[j];

                                ReadIndex(&OffsetData.MorphIndex, InOutBufferCursor, IndexType::Morph, HeaderData.MorphIndexSize);
                                ReadBuffer(&OffsetData.Influence, InOutBufferCursor, sizeof(OffsetData.Influence));
                            }
                        }
                        break;
                    case MorphData::MorphType::Impulse:
                        {
                            Offsets = new MorphData::OffsetImpulse[MorphData.OffsetCount];
                            memset(Offsets, 0, sizeof(MorphData::OffsetImpulse) * MorphData.OffsetCount);

                            for (int j = 0; j < MorphData.OffsetCount; ++j)
                            {
                                auto& OffsetData = ((MorphData::OffsetImpulse*)Offsets)[j];

                                ReadIndex(&OffsetData.RigidbodyIndex, InOutBufferCursor, IndexType::Rigidbody, HeaderData.RigidbodyIndexSize);
                                ReadBuffer(&OffsetData.LocalFlag, InOutBufferCursor, sizeof(OffsetData.LocalFlag));
                                ReadBuffer(&OffsetData.MovementSpeed, InOutBufferCursor, sizeof(OffsetData.MovementSpeed));
                                ReadBuffer(&OffsetData.RotationTorque, InOutBufferCursor, sizeof(OffsetData.RotationTorque));
                            }
                        }
                        break;
                }
            }
        }
    }

    void PMXMeshData::ReadDisplayFrames(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&DisplayFrameCount, InOutBufferCursor, sizeof(DisplayFrameCount));

        if (DisplayFrameCount <= 0)
            return;

        ArrayDisplayFrame = AllocDataArray<DisplayFrameData>(DisplayFrameCount);

        for (int i = 0; i < DisplayFrameCount; ++i)
        {
            DisplayFrameData& DisplayFrameData = ArrayDisplayFrame[i];

            ReadText(&DisplayFrameData.NameLocal, InOutBufferCursor);
            ReadText(&DisplayFrameData.NameUniversal, InOutBufferCursor);

            ReadBuffer(&DisplayFrameData.SpecialFlag, InOutBufferCursor, sizeof(DisplayFrameData.SpecialFlag));

            ReadBuffer(&DisplayFrameData.FrameCount, InOutBufferCursor, sizeof(DisplayFrameData.FrameCount));

            if (DisplayFrameData.FrameCount > 0)
            {
                DisplayFrameData.ArrayFrame = new DisplayFrameData::Frame[DisplayFrameData.FrameCount];
                memset(DisplayFrameData.ArrayFrame, 0, sizeof(DisplayFrameData::Frame) * DisplayFrameData.FrameCount);

                for (int j = 0; j < DisplayFrameData.FrameCount; ++j)
                {
                    DisplayFrameData::Frame& FrameData = DisplayFrameData.ArrayFrame[j];

                    ReadBuffer(&FrameData.Type, InOutBufferCursor, sizeof(FrameData.Type));

                    switch (FrameData.Type)
                    {
                        case DisplayFrameData::Frame::FrameType::Bone:
                            ReadIndex(&FrameData.Index, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);
                            break;

                        case DisplayFrameData::Frame::FrameType::Morph:
                            ReadIndex(&FrameData.Index, InOutBufferCursor, IndexType::Morph, HeaderData.MorphIndexSize);
                            break;
                    }
                }
            }
        }
    }

    void PMXMeshData::ReadRigidbodies(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&RigidbodyCount, InOutBufferCursor, sizeof(RigidbodyCount));

        if (RigidbodyCount <= 0)
            return;

        ArrayRigidbody = AllocDataArray<RigidbodyData>(RigidbodyCount);

        for (int i = 0, max = RigidbodyCount; i < max; ++i)
        {
            RigidbodyData& RigidbodyData = ArrayRigidbody[i];

            ReadText(&RigidbodyData.NameLocal, InOutBufferCursor);
            ReadText(&RigidbodyData.NameUniversal, InOutBufferCursor);

            ReadIndex(&RigidbodyData.BoneIndexRelated, InOutBufferCursor, IndexType::Bone, HeaderData.BoneIndexSize);

            ReadBuffer(&RigidbodyData.GroupID, InOutBufferCursor, sizeof(RigidbodyData.GroupID));
            ReadBuffer(&RigidbodyData.NonCollisionGroupMask, InOutBufferCursor, sizeof(RigidbodyData.NonCollisionGroupMask));

            ReadBuffer(&RigidbodyData.ShapeType, InOutBufferCursor, sizeof(RigidbodyData.ShapeType));
            ReadBuffer(&RigidbodyData.ShapeSize, InOutBufferCursor, sizeof(RigidbodyData.ShapeSize));
            ReadBuffer(&RigidbodyData.ShapePosition, InOutBufferCursor, sizeof(RigidbodyData.ShapePosition));
            ReadBuffer(&RigidbodyData.ShapeRotation, InOutBufferCursor, sizeof(RigidbodyData.ShapeRotation));

            ReadBuffer(&RigidbodyData.Mass, InOutBufferCursor, sizeof(RigidbodyData.Mass));
            ReadBuffer(&RigidbodyData.MoveAttenuation, InOutBufferCursor, sizeof(RigidbodyData.MoveAttenuation));
            ReadBuffer(&RigidbodyData.RotationDamping, InOutBufferCursor, sizeof(RigidbodyData.RotationDamping));
            ReadBuffer(&RigidbodyData.Repulsion, InOutBufferCursor, sizeof(RigidbodyData.Repulsion));
            ReadBuffer(&RigidbodyData.FrictionForce, InOutBufferCursor, sizeof(RigidbodyData.FrictionForce));
            ReadBuffer(&RigidbodyData.PhysicsMode, InOutBufferCursor, sizeof(RigidbodyData.PhysicsMode));
        }
    }

    void PMXMeshData::ReadJoints(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&JointCount, InOutBufferCursor, sizeof(JointCount));

        if (JointCount <= 0)
            return;

        ArrayJoint = AllocDataArray<JointData>(JointCount);

        for (int i = 0, max = JointCount; i < max; ++i)
        {
            JointData& JointData = ArrayJoint[i];

            ReadText(&JointData.NameLocal, InOutBufferCursor);
            ReadText(&JointData.NameUniversal, InOutBufferCursor);

            ReadBuffer(&JointData.Type, InOutBufferCursor, sizeof(JointData.Type));
            ReadIndex(&JointData.RigidbodyIndexA, InOutBufferCursor, IndexType::Rigidbody, HeaderData.RigidbodyIndexSize);
            ReadIndex(&JointData.RigidbodyIndexB, InOutBufferCursor, IndexType::Rigidbody, HeaderData.RigidbodyIndexSize);
            ReadBuffer(&JointData.Position, InOutBufferCursor, sizeof(JointData.Position));
            ReadBuffer(&JointData.Rotation, InOutBufferCursor, sizeof(JointData.Rotation));
            ReadBuffer(&JointData.PositionMin, InOutBufferCursor, sizeof(JointData.PositionMin));
            ReadBuffer(&JointData.PositionMax, InOutBufferCursor, sizeof(JointData.PositionMax));
            ReadBuffer(&JointData.RotationMin, InOutBufferCursor, sizeof(JointData.RotationMin));
            ReadBuffer(&JointData.RotationMax, InOutBufferCursor, sizeof(JointData.RotationMax));
            ReadBuffer(&JointData.PositionSpring, InOutBufferCursor, sizeof(JointData.PositionSpring));
            ReadBuffer(&JointData.RotationSpring, InOutBufferCursor, sizeof(JointData.RotationSpring));
        }
    }

    void PMXMeshData::ReadSoftBodies(const Byte*& InOutBufferCursor)
    {
        ReadBuffer(&SoftBodyCount, InOutBufferCursor, sizeof(SoftBodyCount));

        if (SoftBodyCount <= 0)
            return;

        ArraySoftBody = AllocDataArray<SoftBodyData>(SoftBodyCount);

        for (int i = 0, max = SoftBodyCount; i < max; ++i)
        {
            SoftBodyData& SoftBodyData = ArraySoftBody[i];

            ReadText(&SoftBodyData.NameLocal, InOutBufferCursor);
            ReadText(&SoftBodyData.NameUniversal, InOutBufferCursor);
            ReadBuffer(&SoftBodyData.Shape, InOutBufferCursor, sizeof(SoftBodyData.Shape));
            ReadIndex(&SoftBodyData.MaterialIndex, InOutBufferCursor, IndexType::Material, HeaderData.MaterialIndexSize);
            ReadBuffer(&SoftBodyData.Group, InOutBufferCursor, sizeof(SoftBodyData.Group));
            ReadBuffer(&SoftBodyData.NonCollisionGroupMask, InOutBufferCursor, sizeof(SoftBodyData.NonCollisionGroupMask));
            ReadBuffer(&SoftBodyData.B_LinkCreateDistance, InOutBufferCursor, sizeof(SoftBodyData.B_LinkCreateDistance));
            ReadBuffer(&SoftBodyData.NumberOfClusters, InOutBufferCursor, sizeof(SoftBodyData.NumberOfClusters));
            ReadBuffer(&SoftBodyData.TotalMass, InOutBufferCursor, sizeof(SoftBodyData.TotalMass));
            ReadBuffer(&SoftBodyData.CollisionMargin, InOutBufferCursor, sizeof(SoftBodyData.CollisionMargin));
            ReadBuffer(&SoftBodyData.AerodynamicsModel, InOutBufferCursor, sizeof(SoftBodyData.AerodynamicsModel));
            ReadBuffer(&SoftBodyData.ConfigVCF, InOutBufferCursor, sizeof(SoftBodyData.ConfigVCF));
            ReadBuffer(&SoftBodyData.ConfigDP, InOutBufferCursor, sizeof(SoftBodyData.ConfigDP));
            ReadBuffer(&SoftBodyData.ConfigDG, InOutBufferCursor, sizeof(SoftBodyData.ConfigDG));
            ReadBuffer(&SoftBodyData.ConfigLF, InOutBufferCursor, sizeof(SoftBodyData.ConfigLF));
            ReadBuffer(&SoftBodyData.ConfigPR, InOutBufferCursor, sizeof(SoftBodyData.ConfigPR));
            ReadBuffer(&SoftBodyData.ConfigVC, InOutBufferCursor, sizeof(SoftBodyData.ConfigVC));
            ReadBuffer(&SoftBodyData.ConfigDF, InOutBufferCursor, sizeof(SoftBodyData.ConfigDF));
            ReadBuffer(&SoftBodyData.ConfigMT, InOutBufferCursor, sizeof(SoftBodyData.ConfigMT));
            ReadBuffer(&SoftBodyData.ConfigCHR, InOutBufferCursor, sizeof(SoftBodyData.ConfigCHR));
            ReadBuffer(&SoftBodyData.ConfigKHR, InOutBufferCursor, sizeof(SoftBodyData.ConfigKHR));
            ReadBuffer(&SoftBodyData.ConfigSHR, InOutBufferCursor, sizeof(SoftBodyData.ConfigSHR));
            ReadBuffer(&SoftBodyData.ConfigAHR, InOutBufferCursor, sizeof(SoftBodyData.ConfigAHR));
            ReadBuffer(&SoftBodyData.ClusterSRHR_CL, InOutBufferCursor, sizeof(SoftBodyData.ClusterSRHR_CL));
            ReadBuffer(&SoftBodyData.ClusterSKHR_CL, InOutBufferCursor, sizeof(SoftBodyData.ClusterSKHR_CL));
            ReadBuffer(&SoftBodyData.ClusterSSHR_CL, InOutBufferCursor, sizeof(SoftBodyData.ClusterSSHR_CL));
            ReadBuffer(&SoftBodyData.ClusterSR_SPLT_CL, InOutBufferCursor, sizeof(SoftBodyData.ClusterSR_SPLT_CL));
            ReadBuffer(&SoftBodyData.ClusterSK_SPLT_CL, InOutBufferCursor, sizeof(SoftBodyData.ClusterSK_SPLT_CL));
            ReadBuffer(&SoftBodyData.ClusterSS_SPLT_CL, InOutBufferCursor, sizeof(SoftBodyData.ClusterSS_SPLT_CL));
            ReadBuffer(&SoftBodyData.InterationV_IT, InOutBufferCursor, sizeof(SoftBodyData.InterationV_IT));
            ReadBuffer(&SoftBodyData.InterationP_IT, InOutBufferCursor, sizeof(SoftBodyData.InterationP_IT));
            ReadBuffer(&SoftBodyData.InterationD_IT, InOutBufferCursor, sizeof(SoftBodyData.InterationD_IT));
            ReadBuffer(&SoftBodyData.InterationC_IT, InOutBufferCursor, sizeof(SoftBodyData.InterationC_IT));
            ReadBuffer(&SoftBodyData.MaterialLST, InOutBufferCursor, sizeof(SoftBodyData.MaterialLST));
            ReadBuffer(&SoftBodyData.MaterialAST, InOutBufferCursor, sizeof(SoftBodyData.MaterialAST));
            ReadBuffer(&SoftBodyData.MaterialVST, InOutBufferCursor, sizeof(SoftBodyData.MaterialVST));

            ReadBuffer(&SoftBodyData.AnchorRigidbodyCount, InOutBufferCursor, sizeof(SoftBodyData.AnchorRigidbodyCount));
            SoftBodyData.ArrayAnchorRigidbody = AllocDataArray<SoftBodyData::AnchorRigidbody>(SoftBodyData.AnchorRigidbodyCount);
            ReadBuffer(SoftBodyData.ArrayAnchorRigidbody, InOutBufferCursor, sizeof(SoftBodyData::AnchorRigidbody)* SoftBodyData.AnchorRigidbodyCount);

            ReadBuffer(&SoftBodyData.VertexPinCount, InOutBufferCursor, sizeof(SoftBodyData.VertexPinCount));
            SoftBodyData.ArrayVertexPin = AllocDataArray<SoftBodyData::VertexPin>(SoftBodyData.VertexPinCount);
            ReadBuffer(SoftBodyData.ArrayVertexPin, InOutBufferCursor, sizeof(SoftBodyData::VertexPin) * SoftBodyData.VertexPinCount);
        }
    }
}
