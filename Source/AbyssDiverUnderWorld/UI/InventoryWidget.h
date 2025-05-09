// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UWrapBox;
class UADInventoryComponent;
class UInventorySlotWidget;
enum class EItemType : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Function
public:
	UFUNCTION()
	void RefrashInventoryWidget(EItemType ItemType);
	void InitializeSlots();
	void InventoryWidgetDelegateBind();
	void SetInventoryInfo(int8 Size, EItemType Type, UADInventoryComponent* InventoryComponent);
protected:
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox;
	TObjectPtr<UADInventoryComponent> InventoryComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> SlotWidgetClass;

private:
	int8 InventorySize;
	EItemType InventoryItemType;
#pragma endregion

};
