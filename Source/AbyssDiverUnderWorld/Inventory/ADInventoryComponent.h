// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Container/FStructContainer.h"
#include "Interactable/Item/Component/EquipUseComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ADInventoryComponent.generated.h"


enum class EItemType : uint8;
enum class ESFX : uint8;
class UToggleWidget;
class UDataTableSubsystem;
class USoundSubsystem;
class AADUseItem;
class UChargeBatteryWidget;
enum class EChargeBatteryType;
class AUnderwaterCharacter;
class UUseStrategy;
class UNiagaraSystem;

#define LOGINVEN(Verbosity, Format, ...) UE_LOG(InventoryLog, Verbosity, TEXT("[%s] %s(%s) %s"), LOG_NETMODEINFO, ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(InventoryLog, Log, All);

DECLARE_MULTICAST_DELEGATE(FInventoryUpdateDelegate);
DECLARE_MULTICAST_DELEGATE(FBatteryUpdateDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(FInventoryInfoUpdateDelegate, int32, int32);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UADInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UADInventoryComponent();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Method
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void S_UseInventoryItem(EItemType ItemType = EItemType::Equipment, uint8 SlotIndex = 0, bool bIgnoreCoolTime = false);
	void S_UseInventoryItem_Implementation(EItemType ItemType = EItemType::Equipment, uint8 SlotIndex = 0, bool bIgnoreCoolTime = false);

	UFUNCTION(Server, Reliable)
	void S_TransferSlots(EItemType SlotType, uint8 FromIndex, uint8 ToIndex);
	void S_TransferSlots_Implementation(EItemType SlotType, uint8 FromIndex, uint8 ToIndex);

	UFUNCTION(Server, Reliable)
	void S_RemoveBySlotIndex(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction);
	void S_RemoveBySlotIndex_Implementation(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction);

	UFUNCTION(Server, Reliable)
	void S_EquipmentChargeBattery(EChargeBatteryType ItemChargeBatteryType, int8 Amount);
	void S_EquipmentChargeBattery_Implementation(EChargeBatteryType ItemChargeBatteryType, int8 Amount);

	UFUNCTION(Server, Reliable)
	void S_UseBatteryAmount(int8 Amount);
	void S_UseBatteryAmount_Implementation(int8 Amount);

	UFUNCTION(NetMulticast, Reliable)
	void M_SpawnItemEffect(ESFX SFXType, UNiagaraSystem* VFX, FVector SpawnLocation);
	void M_SpawnItemEffect_Implementation(ESFX SFXType, UNiagaraSystem* VFX, FVector SpawnLocation);

	UFUNCTION(Client, Reliable)
	void C_InventoryPlaySound(ESFX SFXType);
	void C_InventoryPlaySound_Implementation(ESFX SFXType);

	UFUNCTION(Client, Reliable)
	void C_SpawnItemEffect();
	void C_SpawnItemEffect_Implementation();

	UFUNCTION(Client, Reliable)
	void C_SetButtonActive(EChargeBatteryType ItemChargeBatteryType, bool bClientIsActive, int16 ClientAmount);
	void C_SetButtonActive_Implementation(EChargeBatteryType ItemChargeBatteryType, bool bClientIsActive, int16 ClientAmount);

	UFUNCTION(Client, Reliable)
	void C_UpdateBatteryInfo();
	void C_UpdateBatteryInfo_Implementation();

	UFUNCTION(Client, Reliable)
	void C_OnShieldUseFailed();
	void C_OnShieldUseFailed_Implementation();

	UFUNCTION(Client, Reliable)
	void C_SetEquipBatteryAmount(EChargeBatteryType ItemChargeBatteryType);
	void C_SetEquipBatteryAmount_Implementation(EChargeBatteryType ItemChargeBatteryType);

	UFUNCTION(BlueprintCallable)
	void InventoryInitialize();

	UFUNCTION(BlueprintCallable)
	bool AddInventoryItem(const FItemData& ItemData);

	UFUNCTION(BlueprintCallable)
	void ShowInventory(); 
	UFUNCTION(BlueprintCallable)
	void HideInventory();

	UFUNCTION()
	void OnRep_InventoryList();
	
	UFUNCTION()
	void OnRep_CurrentEquipItem();

	int8 FindItemIndexByName(FName ItemName); //아이템 이름으로 InventoryList 인덱스 반환 (빈슬롯이 없으면 -1 반환)
	int8 FindItemIndexByID(int8 ItemID); //빈슬롯이 없으면 - 1 반환
	void RemoveBySlotIndex(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction);
	void ClientRequestInventoryInitialize();
	void InventoryUIUpdate();
	void CopyInventoryFrom(UADInventoryComponent* Source);
	void InventoryMarkArrayDirty();
	void CheckItemsForBattery();
	void PlayEquipAnimation(AUnderwaterCharacter* Character, bool bIsHarpoon);
	void Equip(FItemData& ItemData, int8 SlotIndex);
	void UnEquip();


	FInventoryUpdateDelegate InventoryUpdateDelegate;
	FBatteryUpdateDelegate BatteryUpdateDelegate;
	FInventoryInfoUpdateDelegate InventoryInfoUpdateDelegate;

private:
	int8 GetTypeInventoryEmptyIndex(EItemType ItemType); //빈슬롯이 없으면 -1 반환
	FVector GetDropLocation();

	int8 GetInventoryIndexByTypeAndSlotIndex(EItemType Type, int8 SlotIndex); //못 찾으면 -1 반환
	void SetEquipInfo(int8 TypeInventoryIndex, AADUseItem* SpawnItem);
	
	void DropItem(FItemData& ItemData);

	void OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo);
	void RebuildIndexMap();
	void OnUseCoolTimeEnd(); //아이템 사용 지연
	void PrintLogInventoryData();
	void TryCachedDiver();

#pragma endregion
	
#pragma region Variable
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UToggleWidget> ToggleWidgetClass;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_InventoryList)
	FInventoryList InventoryList; // 실제 아이템 데이터 저장
	FItemData EmptyItem;
	UPROPERTY(Replicated)
	int32 TotalWeight;
	UPROPERTY(Replicated)
	int32 TotalPrice;
	UPROPERTY(Replicated)
	int8 CurrentEquipmentSlotIndex;
	UPROPERTY(Replicated)
	TObjectPtr<AADUseItem> CurrentEquipmentInstance;
	UPROPERTY(Replicated)
	TObjectPtr<AUnderwaterCharacter> CachedDiver;
	UPROPERTY(Replicated)
	uint8 bIsWeapon : 1 = false;
	UPROPERTY(Replicated)
	EEquipmentType EquipmentType;
	uint8 bHasNoAnimation : 1 = false;
	UPROPERTY()
	TObjectPtr<UToggleWidget> ToggleWidgetInstance;

	int32 WeightMax;
	uint8 bCanUseItem : 1;
	

	TMap<EItemType, TArray<int8>> InventoryIndexMapByType;
	TArray<int8> InventorySizeByType;
	TObjectPtr<UDataTableSubsystem> DataTableSubsystem; 
	TObjectPtr<USoundSubsystem> SoundSubsystem;
	TObjectPtr<UChargeBatteryWidget> ChargeBatteryWidget;
	TObjectPtr <UNiagaraSystem> OxygenRefillEffect;
	TObjectPtr <UNiagaraSystem> DropItemEffect;

	UPROPERTY(EditAnywhere, Category = "Harpoon")
	TObjectPtr<UAnimMontage> HarpoonDrawMontage;

	UPROPERTY(EditAnywhere, Category = "DPV")
	TObjectPtr<UAnimMontage> DPVDrawMontage;

	UPROPERTY(EditAnywhere, Category = "Hammer")
	TObjectPtr<UAnimMontage> HammerDrawMontage;
	
	//UPROPERTY()
	//TObjectPtr<AUnderwaterCharacter> CachedDiver;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentEquipItem)
	TObjectPtr<AActor> CurrentEquipItem = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> PrevEquipItem = nullptr;

	const FName HarpoonSocketName = TEXT("Harpoon");
	const FName DPVSocketName = TEXT("DPV");
	const FName FlareSocketName = TEXT("Flare");
	const FName ShotgunSocketName = TEXT("Shotgun");
	const FName MineSocketName = TEXT("MineController");
	const FName HammerSocketName = TEXT("ToyHammer");
#pragma endregion


#pragma region Getter, Setter
public:
	int16 GetTotalWeight() const { return TotalWeight; }
	int16 GetTotalPrice() const { return TotalPrice; }
	uint8 GetSlotIndex() const { return CurrentEquipmentSlotIndex; }
	bool HasEquippedItem()      const { return CurrentEquipmentSlotIndex != INDEX_NONE; }

	const FItemData* GetInventoryItemData(FName ItemNameToFind); //이름으로 아이템 데이터 반환
	const FItemData& GetEquipmentItemDataByIndex(int8 KeyNum) { return InventoryList.Items[InventoryIndexMapByType[EItemType::Equipment][KeyNum]]; }; //타입별 인벤토리 슬롯 값으로 아이템 데이터 반환
	FItemData* GetCurrentEquipmentItemData(); // 현재 장착한 무기 아이템 데이터
	FItemData* GetEditableItemDataByName(FName ItemNameToEdit);

	const FInventoryList& GetInventoryList() { return InventoryList; } 
	bool TryGiveAmmoToEquipment(EBulletType BulletType, int32 AmountPerPickup);

	const TArray<int8>& GetInventoryIndexesByType(EItemType ItemType) const { return InventoryIndexMapByType[ItemType]; } //타입별 인벤토리에 저장된 InventoryList 인벤토리 인덱스 배열 반환
	const TArray<int8>& GetInventorySizeByType() const { return InventorySizeByType; } //인벤토리 사이즈 배열 반환

	void SetChargeBatteryInstance(UChargeBatteryWidget* BatteryWidget);

	UToggleWidget* GetToggleWidgetInstance() const { return ToggleWidgetInstance; } //인벤토리 위젯 인스턴스 반환

private:
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion
};
