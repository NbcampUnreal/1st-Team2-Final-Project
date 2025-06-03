#include "UI/InteractionDescriptionWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UInteractionDescriptionWidget::HandleFocus(AActor* Actor, FString Description)
{
    if (TextAction)
    {
        FText Label = FText::FromString(Description);
        TextAction->SetText(Label);
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
    PlayAnimation(OutAnim);


    FTimerHandle RemoveTimerHandle;
    float RemoveDelay = OutAnim->GetEndTime();
    GetWorld()->GetTimerManager().SetTimer(RemoveTimerHandle,
        FTimerDelegate::CreateLambda([this]() { 
            if (IsInViewport())
            {
                RemoveFromParent();
                UE_LOG(LogTemp, Log, TEXT("Remove from ViewPort"));
            }
            }), RemoveDelay, false);

}
