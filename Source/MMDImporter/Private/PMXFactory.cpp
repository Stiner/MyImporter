// Practice Unreal by Stiner
#include "PMXFactory.h"
#include "MMDImporter/Common/PMXMeshData.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Serialization/ArchiveCookData.h"

USkeletalMesh* CreateSkeletalMesh(UObject* Outer, USkeleton* Skeleon)
{
    USkeletalMesh* NewSkeletalMesh = NewObject<USkeletalMesh>(Outer);
    NewSkeletalMesh->SetFlags(RF_Public | RF_Standalone);

    FSkeletalMeshLODModel* LODModel = new FSkeletalMeshLODModel();

    TArray<FSoftSkinVertex> Vertices;
    {
        FSoftSkinVertex Vertex1;
        Vertex1.Position = FVector3f(0.0f, 0.0f, 0.0f); // 위치 설정
        Vertex1.TangentX = FVector3f(1.0f, 0.0f, 0.0f); // 탄젠트 설정
        Vertex1.TangentZ = FVector3f(0.0f, 0.0f, 1.0f); // 노멀 설정
        Vertex1.UVs[0] = FVector2f(0.0f, 0.0f);       // UV 설정
        Vertices.Add(Vertex1);

        FSoftSkinVertex Vertex2;
        Vertex2.Position = FVector3f(100.0f, 0.0f, 0.0f);
        Vertex2.TangentX = FVector3f(1.0f, 0.0f, 0.0f);
        Vertex2.TangentZ = FVector3f(0.0f, 0.0f, 1.0f);
        Vertex2.UVs[0] = FVector2f(1.0f, 0.0f);
        Vertices.Add(Vertex2);

        FSoftSkinVertex Vertex3;
        Vertex3.Position = FVector3f(0.0f, 100.0f, 0.0f);
        Vertex3.TangentX = FVector3f(1.0f, 0.0f, 0.0f);
        Vertex3.TangentZ = FVector3f(0.0f, 0.0f, 1.0f);
        Vertex3.UVs[0] = FVector2f(0.0f, 1.0f);
        Vertices.Add(Vertex3);
    }

    LODModel->NumVertices = Vertices.Num();
    
    TArray<uint32> Indices = { 0, 1, 2 };
    LODModel->IndexBuffer = Indices;

    FSkelMeshSection Section;
    Section.BaseIndex = 0;
    Section.NumTriangles = 1;
    Section.MaterialIndex = 0;
    LODModel->Sections.Add(Section);

    TArray<FMatrix> RefSkeletonPose;
    RefSkeletonPose.Add(FMatrix::Identity);




    FSkeletalMeshLODRenderData* LODRenderData = new FSkeletalMeshLODRenderData();
    LODRenderData->BuildFromLODModel(LODModel);


    NewSkeletalMesh->GetResourceForRendering()->LODRenderData.Add(LODRenderData);

    return NewSkeletalMesh;
}

bool SaveSkeletalMeshAsAsset(USkeletalMesh* SkeletalMesh, const FString& AssetPath)
{
    if (IsValid(SkeletalMesh) == false)
        return false;

    FString PackageName = FPaths::Combine(TEXT("/Game"), AssetPath);
    UPackage* NewPackage = CreatePackage(*PackageName);
    if (IsValid(NewPackage) == false)
        return false;

    SkeletalMesh->Rename(*FPaths::GetBaseFilename(AssetPath), NewPackage);

    FAssetRegistryModule::AssetCreated(SkeletalMesh);
    NewPackage->MarkPackageDirty();

    FString FullPath = FPaths::ProjectContentDir() + AssetPath + TEXT(".uasset");

    return UPackage::SavePackage(NewPackage, SkeletalMesh, RF_Public | RF_Standalone, *FullPath, GError, nullptr, true, true, SAVE_NoError);
}


UPMXFactory::UPMXFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Formats.Empty();
    Formats.Add(TEXT("pmx;PMX meshes and animations"));
    
    bCreateNew = false;
    bText = false;
    bEditorImport = true;
}

UObject* UPMXFactory::FactoryCreateBinary(
    UClass* InClass,
    UObject* InParent,
    FName InName,
    EObjectFlags Flags,
    UObject* Context,
    const TCHAR* Type,
    const uint8*& Buffer,
    const uint8* BufferEnd,
    FFeedbackContext* Warn,
    bool& bOutOperationCanceled)
{
    


    

    return nullptr;
}

UClass* UPMXFactory::ResolveSupportedClass()
{
    return UPMXFactory::StaticClass();
}
