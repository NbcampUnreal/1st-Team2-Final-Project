#include "UI/InteractionDescriptionWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Interactable/Item/ADItemBase.h"
#include "Interactable/Item/ADExchangeableItem.h"

void UInteractionDescriptionWidget::HandleFocus(AActor* Actor, FString Description)
{  
    if (!TextAction || !NameText || !PriceText)
        return;

    FText Label = FText::FromString(Description);
    NameText->SetVisibility(ESlateVisibility::Collapsed);
    PriceText->SetVisibility(ESlateVisibility::Collapsed);
    TextAction->SetText(Label);
     


    if (AADItemBase* BaseItem = Cast<AADItemBase>(Actor))
    {
        FName Name = BaseItem->GetItemName();
        int32 Price = BaseItem->GetItemPrice();

        FText TextName = FText::FromName(Name);
        NameText->SetText(TextName);
        NameText->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Warning, TEXT("NameText is Visible"));

        if (Price != 0)
        {
            FText TextPrice = FText::AsNumber(Price);
            PriceText->SetText(TextPrice);
            PriceText->SetVisibility(ESlateVisibility::Visible);
            UE_LOG(LogTemp, Warning, TEXT("PriceText is Visible"));
        }
    }
    PlayAnimation(InAnim);

    if (!IsInViewport())
    {
        AddToViewport();
        UE_LOG(LogTemp, Log, TEXT("Add to ViewPort"));
    }
}

void UInteractionDescriptionWidget::HandleFocusLost()
{
    StopAllAnimations();
    RemoveFromParent();

}
