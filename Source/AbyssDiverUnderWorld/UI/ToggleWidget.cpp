#include "UI/ToggleWidget.h"
#include "InventoryWidget.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/RichTextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/WidgetAnimation.h"

void UToggleWidget::InitializeInventoriesInfo(UADInventoryComponent* InventoryComp)
{
    const TArray<int8>& InventorySizeByType = InventoryComp->GetInventorySizeByType();
    InventoryComp->InventoryInfoUpdateDelegate.AddUObject(this, &UToggleWidget::RefreshExchangableInventoryInfo);
    RefreshExchangableInventoryInfo(0, 0);

    bInventoryWidgetShowed = false;

    if (EquipmentInventory && ConsumableInventory && ExchangableInventory)
    {
        EquipmentInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Equipment)], EItemType::Equipment, InventoryComp);
        ConsumableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Consumable)], EItemType::Consumable, InventoryComp);
        ExchangableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Exchangable)], EItemType::Exchangable, InventoryComp);
    }
}

void UToggleWidget::RefreshExchangableInventoryInfo(int32 Mass, int32 Price)
{
    if (MassText && PriceText)
    {
        MassText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Mass)));
        PriceText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Price)));
    }
}

void UToggleWidget::PlaySlideAnimation(bool bIsVisible)
{
    if (bIsVisible)
    {
        if (!bInventoryWidgetShowed)
        {
            GetWorld()->GetTimerManager().ClearTimer(HiddenTimerHandle);
            SetVisibility(ESlateVisibility::Visible);
            bInventoryWidgetShowed = true;
            if (SlideIn && !IsAnimationPlaying(SlideIn))
                PlayAnimation(SlideIn);
        }
    }
    else
    {
        if (bInventoryWidgetShowed)
        {
            if (SlideOUt && !IsAnimationPlaying(SlideOUt))
                PlayAnimation(SlideOUt);
            float HiddenDelay = SlideOUt->GetEndTime();
            GetWorld()->GetTimerManager().SetTimer(HiddenTimerHandle,
                FTimerDelegate::CreateLambda([this]() { 
                    SetVisibility(ESlateVisibility::Hidden);
                    bInventoryWidgetShowed = false;
                    }), HiddenDelay, false);
        }
    }
}

