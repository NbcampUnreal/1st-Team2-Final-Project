// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "InventorySlotWidget.h"
#include "Components/WrapBox.h"
#include "Inventory/ADInventoryComponent.h"

void UInventoryWidget::RefreshInventoryWidget()
{
	if (InventoryWrapBox)
	{
		InventoryWrapBox->ClearChildren();
		for (int8 i = 0; i < InventorySize; ++i)
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(PC, SlotWidgetClass);
			int8 InventoryItemIndexByType = InventoryComp->GetInventoryIndexesByType(InventoryItemType)[i];
			if (InventoryItemIndexByType >= 0)
			{
				SlotWidget->SetItemData(InventoryComp->GetInventoryList().Items[InventoryItemIndexByType], i, InventoryComp);
				if (SlotWidget)
				{
					InventoryWrapBox->AddChild(SlotWidget);
				}
			}
			else
			{
				FItemData EmptySlot;
				EmptySlot.ItemType = InventoryItemType;
				if (SlotWidget)
				{
					SlotWidget->SetItemData(EmptySlot, i, InventoryComp);
					InventoryWrapBox->AddChild(SlotWidget);
				}
			}
		}
	}
}

void UInventoryWidget::InitializeSlots()
{
	for (int8 i = 0; i < InventorySize; ++i)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(PC, SlotWidgetClass);
			FItemData EmptySlot;
			EmptySlot.ItemType = InventoryItemType;
			if (SlotWidget)
			{
				SlotWidget->SetItemData(EmptySlot, i, InventoryComp);
				InventoryWrapBox->AddChild(SlotWidget);
			}
		}
	}
}

void UInventoryWidget::InventoryWidgetDelegateBind()
{
	InventoryComp->InventoryUpdateDelegate.AddUObject(this, &UInventoryWidget::RefreshInventoryWidget);
}

void UInventoryWidget::SetInventoryInfo(int8 Size, EItemType Type, UADInventoryComponent* InventoryComponent)
{
	InventorySize = Size;
	InventoryItemType = Type;
	InventoryComp = InventoryComponent;
	InitializeSlots();
	InventoryWidgetDelegateBind();
}
