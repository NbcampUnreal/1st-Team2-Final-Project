// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventorySlotWidget.h"
#include "Components/RichTextBlock.h"
#include "Container/FStructContainer.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "DragPreviewWidget.h"
#include "InventoryDDO.h"
#include "Inventory/ADInventoryComponent.h"

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SlotIndex != -1)
	{
		if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
		{
			FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
			return EventReply.NativeReply;
		}

	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SlotIndex != -1)
	{
		if (SlotType == EItemType::Consumable)
		{
			if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				InventoryComponent->UseInventoryItem(EItemType::Consumable, SlotIndex);
				return FReply::Handled();
			}
		}

	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& Operation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, Operation);

	if (Operation == nullptr)
	{
		UDragPreviewWidget* PreviewWidgetInstance = CreateWidget<UDragPreviewWidget>(this, DragPreviewWidgetClass);
		if (PreviewWidgetInstance)
		{
			if (Image && Image->GetBrush().GetResourceObject())
			{
				PreviewWidgetInstance->SetPreviewInfo(Cast<UTexture2D>(Image->GetBrush().GetResourceObject()));
			}
			UInventoryDDO* DragDropOp = Cast<UInventoryDDO>(UWidgetBlueprintLibrary::CreateDragDropOperation(UInventoryDDO::StaticClass()));
			DragDropOp->DefaultDragVisual = PreviewWidgetInstance;
			DragDropOp->Pivot = EDragPivot::CenterCenter;
			DragDropOp->Payload = this;
			DragDropOp->Index = SlotIndex;
			DragDropOp->Type = SlotType;
			DragDropOp->OnDragCancelled.AddDynamic(this, &UInventorySlotWidget::HandleDragCancelled);
			
			Operation = DragDropOp;
		}
	}
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (InOperation)
	{
		UInventoryDDO* DragDropOp = Cast<UInventoryDDO>(InOperation);
		if (DragDropOp)
		{
			if (DragDropOp->Index != SlotIndex && DragDropOp->Type == SlotType)
			{
				InventoryComponent->TransferSlots(DragDropOp->Index, SlotIndex);
				return true;
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

void UInventorySlotWidget::SetItemData(FItemData ItemInfo, int32 Index, UADInventoryComponent* InventoryComp)
{
	InventoryComponent = InventoryComp;
	SlotType = ItemInfo.ItemType;
	if (!QuantityText && !Image)
		return;
	QuantityText->SetText(FText::FromString(FString::Printf(TEXT("%d"), ItemInfo.Quantity)));
	if (ItemInfo.Quantity == 0 || ItemInfo.ItemType == EItemType::Equipment)
	{
		QuantityText->SetVisibility(ESlateVisibility::Collapsed);
	}
	Image->SetBrushFromTexture(ItemInfo.Thumbnail);

	SlotIndex = Index;
}

void UInventorySlotWidget::HandleDragCancelled(UDragDropOperation* Operation)
{
	InventoryComponent->RemoveInventoryItem(SlotIndex, -1, true);
}
