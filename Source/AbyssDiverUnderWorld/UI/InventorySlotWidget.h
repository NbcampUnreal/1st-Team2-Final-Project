// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

struct FItemData;
enum class EItemType : uint8;
class URichTextBlock;
class UImage;
class UDragPreviewWidget;
class UADInventoryComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Method
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& Operation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	UFUNCTION(BlueprintCallable)
	void SetItemData(FItemData ItemInfo, int32 Index, UADInventoryComponent* InventoryComp);
	UFUNCTION()
	void HandleDragCancelled(UDragDropOperation* Operation);
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDragPreviewWidget> DragPreviewWidgetClass;
private:
	int8 SlotIndex;
	EItemType SlotType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> QuantityText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image;
	TObjectPtr<UADInventoryComponent> InventoryComponent;
#pragma endregion
};
