#include "Shops/ShopWidgets/ShopBuyListSlotWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Shops/ShopBuyListEntryData.h"
#include "Subsystems/SoundSubsystem.h"

#include "Components/TextBlock.h"

void UShopBuyListSlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

    UShopBuyListEntryData* EntryData = Cast<UShopBuyListEntryData>(ListItemObject);
    if (EntryData == nullptr)
    {
        LOGV(Error, TEXT("EntryData == nullptr"));
        return;
    }

    SetSlotImage(EntryData->GetItemImage());
    SetToolTipText(EntryData->GetToolTipText());
    SlotIndex = EntryData->GetSlotIndex();
    ItemCount = EntryData->GetItemCount();
    SetItemCountText(ItemCount);
    SetActiveLockImage(false);

    EntryData->OnEntryUpdatedFromDataDelegate.Broadcast(this);
    LOGV(Error, TEXT("BuyListEntry Updated, Index : %d, Count : %d"), SlotIndex, ItemCount);
}

FReply UShopBuyListSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Replay = UUserWidget::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    LOGV(Log, TEXT("ShopBuyListSlotWidgetClicked, Index : %d"), SlotIndex);
    OnShopItemSlotWidgetClickedDelegate.ExecuteIfBound(SlotIndex);
    GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);

    return Replay;
}

void UShopBuyListSlotWidget::SetItemCountText(int32 NewItemCount)
{
    ItemCountText->SetText(FText::FromString(FString::FromInt(NewItemCount)));
}