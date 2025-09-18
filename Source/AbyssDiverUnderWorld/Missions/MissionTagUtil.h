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
	// ---- Unit ----
	UFUNCTION(BlueprintPure, Category = "TagUtil|Unit")
	static FGameplayTag ToUnitIdTag(EUnitId UnitId);                 // Unit.id.<Tail>

	/** ī�װ� ������ ���� ���� (��: "Shark", "Predator") */
	UFUNCTION(BlueprintPure, Category = "TagUtil|Unit")
	static FGameplayTag ToUnitTypeTagByTail(FName CategoryTail);     // Unit.Type.<Tail>

	UFUNCTION(BlueprintPure, Category = "TagUtil|Unit")
	static bool IsUnitIdTag(const FGameplayTag& Tag);
	UFUNCTION(BlueprintPure, Category = "TagUtil|Unit")
	static bool IsUnitTypeTag(const FGameplayTag& Tag);

	// ---- Item ----
	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static FGameplayTag ToItemIdTag(uint8 ItemId);                 // Item.id.<Tail>

	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static FGameplayTag ToItemTypeTagByTail(FName CategoryTail);     // Item.Type.<Tail>

	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static bool IsItemIdTag(const FGameplayTag& Tag);
	UFUNCTION(BlueprintPure, Category = "TagUtil|Item")
	static bool IsItemTypeTag(const FGameplayTag& Tag);

	// ---- Debug ----
	UFUNCTION(BlueprintPure, Category = "TagUtil|Debug")
	static FString GetTailFromTag(const FGameplayTag& Tag);

private:
	template<typename TEnum>
	static FString EnumToTailString(TEnum Value); // DisplayName �켱, ������ ���� �̸�

	static FGameplayTag RequestTagChecked(const TCHAR* Root, const FString& Tail);

	// ��Ʈ (��ҹ��� ����)
	static constexpr const TCHAR* Root_UnitId() { return TEXT("Unit.id"); }
	static constexpr const TCHAR* Root_UnitType() { return TEXT("Unit.Type"); }
	static constexpr const TCHAR* Root_ItemId() { return TEXT("Item.id"); }
	static constexpr const TCHAR* Root_ItemType() { return TEXT("Item.Type"); }
};
