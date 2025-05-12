// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Container/FStructContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ADInventoryComponent.generated.h"


enum class EItemType : uint8;
class UAllInventoryWidget;
class UDataTableSubsystem;

DECLARE_MULTICAST_DELEGATE(FInventoryUpdateDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(FInventoryInfoUpdateDelegate, int32, int32);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UADInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UADInventoryComponent();
protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

	UFUNCTION(Server, Reliable)
	void S_DropItem(FItemData ItemData);
	void S_DropItem_Implementation(FItemData ItemData);

	UFUNCTION(BlueprintCallable)
	void AddInventoryItem(FItemData ItemData);
	bool RemoveInventoryItem(uint8 InventoryIndex, int8 Count, bool bIsDropAction);

	UFUNCTION(BlueprintCallable)
	void TransferSlots(uint8 FromIndex, uint8 ToIndex);

	//*Remove 테스트 후 넣어야 함
	UFUNCTION(BlueprintCallable)
	void ToggleInventoryShowed(); //추후 나침반이나 서브미션 UI 추가되었을 때 고려 대상
	UFUNCTION(BlueprintCallable)
	void InventoryInitialize();

	FInventoryUpdateDelegate InventoryUpdateDelegate;
	FInventoryInfoUpdateDelegate InventoryInfoUpdateDelegate;

private:
	int8 GetTypeInventoryEmptyIndex(EItemType ItemType);
	int16 FindItemIndexById(FName ItemID);
	void OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo);

	void InventoryUIUpdate();
	FVector GetDropLocation();
	void PrintLogInventoryData();

#pragma endregion
	
#pragma region Variable
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAllInventoryWidget> InventoryWidgetClass;
	
private:
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	int16 TotalWeight;
	int16 TotalPrice;
	int16 WeightMax;

	uint8 bInventoryWidgetShowed : 1;

	TMap<EItemType, TArray<int8>> InventoryIndexMapByType;
	TArray<int8> InventorySizeByType;

	TObjectPtr<UAllInventoryWidget> InventoryWidgetInstance;
	TObjectPtr<UDataTableSubsystem> ItemDataTableSubsystem; 
	TObjectPtr<UDataTable> TestItemDataTable;
#pragma endregion


#pragma region Getter, Setter
public:
	int16 GetTotalWeight() const { return TotalWeight; }
	int16 GetTotalPrice() const { return TotalPrice; }
	const FItemData& GetItemData(FName ItemNameToFind) { return InventoryList.Items[FindItemIndexById(ItemNameToFind)]; };
	const FItemData& GetEquipmentItemDataByIndex(int8 KeyNum) { return InventoryList.Items[InventoryIndexMapByType[EItemType::Equipment][KeyNum]]; };
	const FInventoryList& GetInventoryList() { return InventoryList; }
	const TArray<int8>& GetInventoryIndexesByType(EItemType ItemType) const { return InventoryIndexMapByType[ItemType]; }
	const TArray<int8>& GetInventorySizeByType() const { return InventorySizeByType; }
#pragma endregion
};
