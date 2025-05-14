#include "ShopItemSlotWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Shops/ShopItemEntryData.h"

#include "Components/Image.h"
#include "Components/RichTextBlock.h"

void UShopItemSlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

    UShopItemEntryData* EntryData = Cast<UShopItemEntryData>(ListItemObject);
    if (EntryData == nullptr)
    {
        LOGV(Error, TEXT("EntryData == nullptr"));
        return;
    }

    SetSlotImage(EntryData->GetItemImage());
    SetToolTipText(EntryData->GetToolTipText());
    SlotIndex = EntryData->GetSlotIndex();

    EntryData->OnEntryUpdatedFromDataDelegate.Broadcast(this);
    LOGV(Log, TEXT("%s - EntryUpdated, Index : %d"), *GetName(), SlotIndex);
}

FReply UShopItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Replay =  Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    LOGV(Warning, TEXT("ShopItemSlotWidgetClicked, Index : %d"), SlotIndex);
    OnShopItemSlotWidgetClickedDelegate.ExecuteIfBound(SlotIndex);

    return Replay;
}

void UShopItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    UpdateToolTipVisibility(true);
}

void UShopItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    UpdateToolTipVisibility(false);
}

void UShopItemSlotWidget::UpdateToolTipVisibility(bool bShouldShow)
{
    // 나중에 서서히 등장하는 효과를 넣을 수도 있다고 생각해서 Color로
    FLinearColor Color(1, 1, 1, 1);

    if (bShouldShow)
    {
        Color.A = 1;
        
    }
    else
    {
        Color.A = 0;
    }

    ToolTipImage->SetColorAndOpacity(Color);
    ToolTipTextBlock->SetRenderOpacity(Color.A);
}

void UShopItemSlotWidget::SetSlotImage(UTexture2D* NewTexture)
{
    if (NewTexture == nullptr)
    {
        LOGV(Warning, TEXT("NewTexture == nullptr"));
        return;
    }

    SlotImage->SetBrushFromTexture(NewTexture);
}

void UShopItemSlotWidget::SetToolTipText(const FString& NewText)
{
    ToolTipTextBlock->SetText(FText::FromString(NewText));
}
