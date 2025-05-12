// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AllInventoryWidget.generated.h"

class UInventoryWidget;
class UADInventoryComponent;
class URichTextBlock;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAllInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Function
public:
	void InitializeInventoriesInfo(UADInventoryComponent* InventoryComp);
	void RefreshExchangableInventoryInfo(int32 Mass, int32 Price);
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> EquipmentInventory;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> ConsumableInventory;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> ExchangableInventory;
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> MassText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> PriceText;

#pragma endregion

#pragma region Getter/Setter
public:
	UInventoryWidget* GetEquipmentInventory() const { return EquipmentInventory; }
	UInventoryWidget* GetConsumableInventory() const { return ConsumableInventory; }
	UInventoryWidget* GetExchangableInventory() const { return ExchangableInventory; }
#pragma endregion
};
