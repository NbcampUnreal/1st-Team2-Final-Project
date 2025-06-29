﻿#include "Framework/CreateTeamWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ADGameInstance.h"


//Components
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"

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
    OnBackClickedDelegate.ExecuteIfBound();
}

void UCreateTeamWidget::OnConfirmClicked()
{
    RemoveFromParent();
}

void UCreateTeamWidget::UpdateSelection()
{
}
