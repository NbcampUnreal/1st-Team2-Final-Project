#include "UI/InteractionDescriptionWidget.h"
#include "Components/TextBlock.h"

void UInteractionDescriptionWidget::HandleFocus(AActor* Actor, EInteractionType Type)
{
    if (TextAction)
    {
        FText Label = UEnum::GetDisplayValueAsText(Type);
        TextAction->SetText(Label);
    }


    if (!IsInViewport())
    {
        AddToViewport();
        UE_LOG(LogTemp, Log, TEXT("Add to ViewPort"));
    }
}

void UInteractionDescriptionWidget::HandleFocusLost()
{
    if (IsInViewport())
    {
        RemoveFromParent();
        UE_LOG(LogTemp, Log, TEXT("Remove from ViewPort"));
    }
}
