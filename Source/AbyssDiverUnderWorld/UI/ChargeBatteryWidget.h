// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChargeBatteryWidget.generated.h"

struct FItemData;
class URichTextBlock;
class UADInventoryComponent;
class UEquipUseComponent;
class UButton;

#define LOGB(Verbosity, Format, ...) UE_LOG(InventoryLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(BatteryLog, Log, All);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UChargeBatteryWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Method
public:
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable)
	void StartChargeBattery(FName ItemName);
	UFUNCTION(BlueprintCallable)
	void StopChargeBattery();

	UFUNCTION(BlueprintCallable)
	bool CanCharge();

	void ChargeBatteryAmount();

	void UpdateBatteryInfoDelay();
	void UpdateBatteryInfo();
private:
	void InitializeChargeBatteryWidget();
#pragma endregion

#pragma region Variable
private:
	TObjectPtr<UADInventoryComponent> InventoryComp = nullptr;
	TObjectPtr<UEquipUseComponent> EquipUseComp = nullptr;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> BatteryAmountText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> BatteryNumText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DPVBatteryText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> NVBatteryText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DPVButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NVButton;

	FTimerHandle InitialzieTimerHandle;

	int32 DPVBatteryMax = 0;
	int32 NVBatteryMax = 0;
	uint8 BatteryMax = 0;
	uint8 InitializeNum = 5;

	FName CurrentChargeItem;
	FTimerHandle IncreaseTimerHandle;
#pragma endregion

#pragma region Getter/Setter
public:
	void SetEquipBatteryAmount(FName Name, int32 Amount);
	void SetEquipBatteryButtonActivate(FName Name, bool bActivate);
#pragma endregion
};
