#include "MMDImporterBPLibrary.h"
#include "StaticMeshAttributes.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UMMDImporterBPLibrary::Import(const FString& AssetName)
{
    if (AssetName.IsEmpty())
        return;

    // NOTE: 파일 경로(**/*.uasset) 조합 후 패키지 이름(/Game/**) 생성.
    const FString PackageFileName = FPaths::SetExtension(AssetName, FPackageName::GetAssetPackageExtension());
    const FString PackagePath = FPaths::Combine(FPaths::ProjectContentDir(), PackageFileName);
    const FString PackageName = FPackageName::FilenameToLongPackageName(PackagePath);

    UPackage* NewPackage = CreatePackage(*PackageName);
    if (IsValid(NewPackage) == false)
        return;

    // NOTE: 새 에셋 생성시 반드시 Outer와 RF_Public, RF_Standalone 플래그를 지정 해 줘야 함.
    UObject* NewAsset = CreateMeshAsset(NewPackage, *AssetName, RF_Public | RF_Standalone);
    if (IsValid(NewAsset) == false)
        return;

    FAssetRegistryModule::AssetCreated(NewAsset);

    NewPackage->MarkPackageDirty();

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

    UPackage::SavePackage(NewPackage, NewAsset, *PackagePath, SaveArgs);

    TArray<UObject*> ObjectsToSync({ NewAsset });
    GEditor->SyncBrowserToObjects(ObjectsToSync);
}

UObject* UMMDImporterBPLibrary::CreateMeshAsset(UObject* NewOuter, const TCHAR* NewAssetName, const EObjectFlags NewFlags)
{
    FMeshDescription MeshDesc;

    FStaticMeshAttributes MeshAttributes(MeshDesc);
    MeshAttributes.Register();

    TVertexAttributesRef<FVector3f> Positions = MeshDesc.GetVertexPositions();

    MeshDesc.ReserveNewVertices(4);
    FVertexID v0 = MeshDesc.CreateVertex();
    FVertexID v1 = MeshDesc.CreateVertex();
    FVertexID v2 = MeshDesc.CreateVertex();
    FVertexID v3 = MeshDesc.CreateVertex();

    MeshDesc.ReserveNewVertexInstances(4);
    FVertexInstanceID vi0 = MeshDesc.CreateVertexInstance(v0);
    FVertexInstanceID vi1 = MeshDesc.CreateVertexInstance(v1);
    FVertexInstanceID vi2 = MeshDesc.CreateVertexInstance(v2);
    FVertexInstanceID vi3 = MeshDesc.CreateVertexInstance(v3);

    MeshDesc.ReserveNewUVs(4);
    FUVID uv0 = MeshDesc.CreateUV();
    FUVID uv1 = MeshDesc.CreateUV();
    FUVID uv2 = MeshDesc.CreateUV();
    FUVID uv3 = MeshDesc.CreateUV();

    FPolygonGroupID polygonGroup = MeshDesc.CreatePolygonGroup();

    MeshDesc.ReserveNewPolygons(1);
    MeshDesc.CreatePolygon(polygonGroup, { vi0, vi1, vi2, vi3 });

    Positions = MeshAttributes.GetVertexPositions();
    Positions[0] = FVector3f(-100, -100, 0);
    Positions[1] = FVector3f(-100, 100, 0);
    Positions[2] = FVector3f(100, 100, 0);
    Positions[3] = FVector3f(100, -100, 0);

    TVertexInstanceAttributesRef<FVector3f> normals = MeshAttributes.GetVertexInstanceNormals();
    normals[0] = FVector3f(0, 0, 1);
    normals[1] = FVector3f(0, 0, 1);
    normals[2] = FVector3f(0, 0, 1);
    normals[3] = FVector3f(0, 0, 1);

    TVertexInstanceAttributesRef<FVector2f> uvs = MeshAttributes.GetVertexInstanceUVs();
    uvs[0] = FVector2f(0, 0);
    uvs[1] = FVector2f(0, 1);
    uvs[2] = FVector2f(1, 1);
    uvs[3] = FVector2f(1, 0);

    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(NewOuter, NewAssetName, NewFlags);
    StaticMesh->GetStaticMaterials().Add(FStaticMaterial());

    UStaticMesh::FBuildMeshDescriptionsParams BuildMeshDescParams;
    BuildMeshDescParams.bBuildSimpleCollision = true;

    StaticMesh->NaniteSettings.bEnabled = true;

    StaticMesh->BuildFromMeshDescriptions({ &MeshDesc }, BuildMeshDescParams);

    return StaticMesh;
}
