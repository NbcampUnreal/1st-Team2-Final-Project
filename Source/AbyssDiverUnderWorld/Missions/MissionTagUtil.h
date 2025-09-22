#pragma once

//enum
#include "Character/UnitBase.h"

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MissionTagUtil.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionTagUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// ---- Unit 쪽은 그대로 (UENUM/문자열 tail) ----
	static FGameplayTag ToUnitIdTag(EUnitId UnitId);
	static FGameplayTag ToUnitTypeTagByTail(FName CategoryTail);
	static bool IsUnitIdTag(const FGameplayTag& Tag);
	static bool IsUnitTypeTag(const FGameplayTag& Tag);

	// ---- Item: uint8 기반 ----
	// 1) 숫자 id -> Item.id.<Tail>
	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static FGameplayTag ToItemIdTagById(uint8 ItemId);

	// 2) 카테고리 이름 tail -> Item.Type.<Tail>
	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static FGameplayTag ToItemTypeTagByTail(FName CategoryTail);

	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static bool IsItemIdTag(const FGameplayTag& Tag);
	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static bool IsItemTypeTag(const FGameplayTag& Tag);

	// ---- Item 이름 매핑(선택) ----
	// 예) 0->"CoralSample", 1->"BatteryPack"
	UFUNCTION(BlueprintCallable, Category = "TagUtil|Item")
	static void RegisterItemIdNames(const TArray<FName>& Names); // index = id
	UFUNCTION(BlueprintCallable, Category = "TagUtil|Item")
	static void SetItemIdName(uint8 ItemId, FName Tail);

	// 디버그용
	static FString GetTailFromTag(const FGameplayTag& Tag);

private:
	template<typename TEnum> static FString EnumToTailString(TEnum Value); // (UnitId가 UENUM이면 사용)
	static FGameplayTag RequestTagChecked(const TCHAR* Root, const FString& Tail);
	static constexpr const TCHAR* Root_UnitId() { return TEXT("Unit.id"); }
	static constexpr const TCHAR* Root_UnitType() { return TEXT("Unit.Type"); }
	static constexpr const TCHAR* Root_ItemId() { return TEXT("Item.id"); }
	static constexpr const TCHAR* Root_ItemType() { return TEXT("Item.Type"); }

	// 숫자 id -> tail 이름 캐시
	static TMap<uint8, FName> GItemIdToTail;
};
