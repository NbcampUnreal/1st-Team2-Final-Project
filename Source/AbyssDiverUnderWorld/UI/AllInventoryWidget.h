// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AllInventoryWidget.generated.h"

class UInventoryWidget;
class UADInventoryComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAllInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Function
public:
	void InitializeInventoriesInfo(UADInventoryComponent* InventoryComp);
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> EquipmentInventory;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> ConsumableInventory;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> ExchangableInventory;
#pragma endregion

#pragma region Getter/Setter
public:
	UInventoryWidget* GetEquipmentInventory() const { return EquipmentInventory; }
	UInventoryWidget* GetConsumableInventory() const { return ConsumableInventory; }
	UInventoryWidget* GetExchangableInventory() const { return ExchangableInventory; }
#pragma endregion
};
