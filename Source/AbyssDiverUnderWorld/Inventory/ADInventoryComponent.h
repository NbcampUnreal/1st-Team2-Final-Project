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
class AADUseItem;

UENUM(BlueprintType)
enum class EItemEquipState : uint8
{
	Idle = 0,
	Equip = 1,
	Use = 2,
	Max = 3 UMETA(Hidden)
};

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

	void SetEquipInfo(int8 TypeInventoryIndex, AADUseItem* SpawnItem);

	UFUNCTION(BlueprintCallable)
	void AddInventoryItem(FItemData ItemData);
	bool RemoveInventoryItem(uint8 InventoryIndex, int8 Count, bool bIsDropAction);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void S_UseInventoryItem(EItemType ItemType = EItemType::Equipment, int32 InventoryIndex = 0);
	void S_UseInventoryItem_Implementation(EItemType ItemType = EItemType::Equipment, int32 InventoryIndex = 0);

	UFUNCTION(BlueprintCallable)
	void TransferSlots(uint8 FromIndex, uint8 ToIndex);

	UFUNCTION(BlueprintCallable)
	void ToggleInventoryShowed(); //추후 나침반이나 서브미션 UI 추가되었을 때 고려 대상
	UFUNCTION(BlueprintCallable)
	void InventoryInitialize();
	UFUNCTION()
	void OnRep_InventoryList();

	FInventoryUpdateDelegate InventoryUpdateDelegate;
	FInventoryInfoUpdateDelegate InventoryInfoUpdateDelegate;

private:
	int8 GetTypeInventoryEmptyIndex(EItemType ItemType);
	int16 FindItemIndexById(FName ItemID);
	FItemData CurrentEquipmentItemData();
	FVector GetDropLocation();
	void OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo);
	void InventoryUIUpdate();
	void PrintLogInventoryData();
	void RebuildIndexMap();
	void Equip(FItemData ItemData, int8 Index);
	void UnEquip();
	void OnUseCoolTimeEnd();

#pragma endregion
	
#pragma region Variable
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAllInventoryWidget> InventoryWidgetClass;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_InventoryList)
	FInventoryList InventoryList; // 실제 아이템 데이터 저장
	UPROPERTY(Replicated)
	int32 TotalWeight;
	UPROPERTY(Replicated)
	int32 TotalPrice;
	int32 WeightMax;

	uint8 bInventoryWidgetShowed : 1;
	uint8 bCanUseItem : 1;

	TMap<EItemType, TArray<int8>> InventoryIndexMapByType;
	UPROPERTY(Replicated)
	int8 CurrentEquipmentIndex;
	UPROPERTY(Replicated)
	TObjectPtr<AADUseItem> CurrentEquipmentInstance;
	UPROPERTY(Replicated)
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
