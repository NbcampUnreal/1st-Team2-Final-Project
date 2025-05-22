// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChargeBatteryWidget.generated.h"

struct FItemData;
class URichTextBlock;
class UADInventoryComponent;
class UEquipUseComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UChargeBatteryWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Method
public:
#pragma endregion

#pragma region Variable
	TObjectPtr<UADInventoryComponent> InventoryComp;
	TObjectPtr<UEquipUseComponent> EquipUseComp;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> BatteryAmountText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> BatteryNumText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DPVBatteryText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> NVBatteryText;
#pragma endregion

};
