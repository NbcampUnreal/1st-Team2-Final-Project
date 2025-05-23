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

UCLASS()
class ABYSSDIVERUNDERWORLD_API UChargeBatteryWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Method
public:
	virtual void NativeConstruct() override;

private:
	void InitializeInventory();
#pragma endregion

#pragma region Variable
private:
	TObjectPtr<UADInventoryComponent> InventoryComp = nullptr;

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
	uint8 NVBatteryMax = 0;
	uint8 InitializeNum = 5;
#pragma endregion

#pragma region Getter/Setter
public:
	void SetEquipBatteryAmount(FName Name, int32 Amount);

	void SetBatteryButtonActivate(FName Name, bool bActivate);
#pragma endregion
};
