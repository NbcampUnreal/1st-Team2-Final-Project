#include "Framework/CreateTeamWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ADGameInstance.h"


//Components
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Animation/WidgetAnimation.h"

void UCreateTeamWidget::NativeConstruct()
{
    Super::NativeConstruct();


    if (Button_Public)
        Button_Public->OnClicked.AddDynamic(this, &UCreateTeamWidget::OnPublicClicked);

    if (Button_Private)
        Button_Private->OnClicked.AddDynamic(this, &UCreateTeamWidget::OnPrivateClicked);

    if (Button_Back)
        Button_Back->OnClicked.AddDynamic(this, &UCreateTeamWidget::OnBackClicked);

    if (Button_Confirm)
        Button_Confirm->OnClicked.AddDynamic(this, &UCreateTeamWidget::OnConfirmClicked);
}

void UCreateTeamWidget::OnPublicClicked()
{
	bIsPrivate = false;
    UpdateSelection();
}

void UCreateTeamWidget::OnPrivateClicked()
{
    bIsPrivate = true;
    UpdateSelection();
}

void UCreateTeamWidget::OnBackClicked()
{
    PlayAnimation(OutAnim);
    FTimerHandle RemoveWidgetTimerHandle;
    float RemoveDelay = OutAnim->GetEndTime();
    GetWorld()->GetTimerManager().SetTimer(RemoveWidgetTimerHandle,
        FTimerDelegate::CreateLambda([this]()
            {
                OnBackClickedDelegate.ExecuteIfBound();
            }), RemoveDelay, false);
    
}

void UCreateTeamWidget::OnConfirmClicked()
{
    StopAllAnimations();
    RemoveFromParent();
}

void UCreateTeamWidget::UpdateSelection()
{
}
