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

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void S_UseInventoryItem(EItemType ItemType = EItemType::Equipment, int32 SlotIndex = 0);
	void S_UseInventoryItem_Implementation(EItemType ItemType = EItemType::Equipment, int32 SlotIndex = 0);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void S_InventoryInitialize();
	void S_InventoryInitialize_Implementation();

	UFUNCTION(Server, Reliable)
	void S_TransferSlots(uint8 FromIndex, uint8 ToIndex);
	void S_TransferSlots_Implementation(uint8 FromIndex, uint8 ToIndex);

	UFUNCTION(Server, Reliable)
	void S_RequestRemove(uint8 InventoryIndex, int8 Count, bool bIsDropAction);
	void S_RequestRemove_Implementation(uint8 InventoryIndex, int8 Count, bool bIsDropAction);

	UFUNCTION(Server, Reliable)
	void S_RemoveBySlotIndex(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction);
	void S_RemoveBySlotIndex_Implementation(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction);

	UFUNCTION(BlueprintCallable)
	void InventoryInitialize();

	UFUNCTION(BlueprintCallable)
	void AddInventoryItem(FItemData ItemData);

	UFUNCTION(BlueprintCallable)
	void ToggleInventoryShowed(); //추후 나침반이나 서브미션 UI 추가되었을 때 고려 대상

	UFUNCTION()
	void OnRep_InventoryList();

	int16 FindItemIndexByName(FName ItemID); //아이템 이름으로 InventoryList 인덱스 반환 (빈슬롯이 없으면 -1 반환)
	void RemoveInventoryItem(uint8 InventoryIndex, int8 Count, bool bIsDropAction);
	void RemoveBySlotIndex(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction);
	void ClientRequestInventoryInitialize();
	void InventoryUIUpdate();

	FInventoryUpdateDelegate InventoryUpdateDelegate;
	FInventoryInfoUpdateDelegate InventoryInfoUpdateDelegate;

private:
	int8 GetTypeInventoryEmptyIndex(EItemType ItemType); //빈슬롯이 없으면 -1 반환
	FVector GetDropLocation();

	int8 GetInventoryIndexByTypeAndSlotIndex(EItemType Type, int8 SlotIndex); //못 찾으면 -1 반환
	FItemData GetCurrentEquipmentItemData(); // 현재 장착한 무기 아이템 데이터
	void SetEquipInfo(int8 TypeInventoryIndex, AADUseItem* SpawnItem);
	void Equip(FItemData ItemData, int8 SlotIndex);
	void UnEquip();
	void DropItem(FItemData& ItemData);

	void OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo);
	void RebuildIndexMap();
	void OnUseCoolTimeEnd(); //아이템 사용 지연
	void ServerSideInventoryInitialize();
	void PrintLogInventoryData();

#pragma endregion
	
#pragma region Variable
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAllInventoryWidget> InventoryWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> NightVisionWidgetClass;
	
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
	int8 CurrentEquipmentSlotIndex;
	UPROPERTY(Replicated)
	TObjectPtr<AADUseItem> CurrentEquipmentInstance;
	UPROPERTY(Replicated)
	TArray<int8> InventorySizeByType;

	TObjectPtr<UAllInventoryWidget> InventoryWidgetInstance;
	TObjectPtr<UUserWidget> NightVisionWidgetInstance;
	TObjectPtr<UDataTableSubsystem> DataTableSubsystem; 
#pragma endregion


#pragma region Getter, Setter
public:
	int16 GetTotalWeight() const { return TotalWeight; }
	int16 GetTotalPrice() const { return TotalPrice; }

	const FItemData& GetItemData(FName ItemNameToFind) { return InventoryList.Items[FindItemIndexByName(ItemNameToFind)]; }; //이름으로 아이템 데이터 반환
	const FItemData& GetEquipmentItemDataByIndex(int8 KeyNum) { return InventoryList.Items[InventoryIndexMapByType[EItemType::Equipment][KeyNum]]; }; //타입별 인벤토리 슬롯 값으로 아이템 데이터 반환
	FItemData& CurrentEquipmentItemData(); // 현재 장착한 무기 아이템 데이터

	const FInventoryList& GetInventoryList() { return InventoryList; } 

	const TArray<int8>& GetInventoryIndexesByType(EItemType ItemType) const { return InventoryIndexMapByType[ItemType]; } //타입별 인벤토리에 저장된 InventoryList 인벤토리 인덱스 배열 반환
	const TArray<int8>& GetInventorySizeByType() const { return InventorySizeByType; } //인벤토리 사이즈 배열 반환

#pragma endregion
};
