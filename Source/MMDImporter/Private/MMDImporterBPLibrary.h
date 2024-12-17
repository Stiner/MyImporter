#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MMDImporterBPLibrary.generated.h"

/*
* 함수 라이브러리 클래스.
* 각 함수는 정적이어야 하며 모든 블루프린트에서 호출할 수 있는 블루프린트 노드를 나타냅니다.
*
* 함수를 선언할 때 노드에 대한 메타데이터를 정의할 수 있습니다. 주요 함수 지정자는 BlueprintPure와 BlueprintCallable입니다.
* BlueprintPure - 함수가 소유 객체에 어떤 식으로도 영향을 미치지 않으므로 Exec 핀이 없는 노드를 만듭니다.
* BlueprintCallable - Blueprint에서 실행할 수 있는 함수를 만듭니다. 따라서 Exec 핀이 있습니다.
* DisplayName - 노드의 전체 이름으로, 노드 위에 마우스를 올리고 블루프린트 드롭다운 메뉴에 표시됩니다.
* C++ 함수 이름에서 허용되지 않는 문자를 사용하여 노드 이름을 지정할 수 있습니다.
* CompactNodeTitle - 노드에 나타나는 단어입니다.
* 키워드 - Blueprint 드롭다운 메뉴를 사용하여 노드를 검색할 때 노드를 찾는 데 도움이 되는 키워드 목록입니다.
* 좋은 예로 "Print String" 노드가 있는데, 키워드 "log"를 사용하여 찾을 수도 있습니다.
* 카테고리 - 블루프린트 드롭다운 메뉴에서 노드가 속할 카테고리입니다.
*
* 사용자 지정 블루프린트 노드에 대한 자세한 내용은 설명서를 참조하세요.
* https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

class UStaticMesh;

UCLASS()
class UMMDImporterBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "MMD Importer")
    static void Import(const FString& AssetName);

protected:
    static UObject* CreateMeshAsset(UObject* OuterObject, const TCHAR* AssetName, EObjectFlags NewFlags);
};
