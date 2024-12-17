// Practice Unreal by Stiner
#include "PMXFactory.h"

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
