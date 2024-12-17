// Practice Unreal by Stiner

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PMXFactory.generated.h"

/**
 * PMX 임포트 처리
 */
UCLASS()
class UPMXFactory : public UFactory
{
    GENERATED_BODY()

public:
    UPMXFactory(const FObjectInitializer& ObjectInitializer);

    virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

    virtual UClass* ResolveSupportedClass() override; 
};
