// Fill out your copyright notice in the Description page of Project Settings.


#include "SpectatorHUDWidget.h"

#include "Character/UnderwaterCharacter.h"
#include "Components/RichTextBlock.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADPlayerState.h"

void USpectatorHUDWidget::UpdateSpectatorTargetName(const FString& TargetName)
{
	if (SpectateTargetNameText)
	{
		FText FormattedText = FText::Format(
			FText::FromString(TEXT("Spectating: {0}")),
			FText::FromString(TargetName)
		);
		SpectateTargetNameText->SetText(FormattedText);
	}
}

void USpectatorHUDWidget::BindWidget(class AADPlayerController* PlayerController)
{
	PlayerController->OnTargetViewChanged.AddDynamic(this, &USpectatorHUDWidget::OnTargetViewChanged);
}

void USpectatorHUDWidget::OnTargetViewChanged(AActor* NewViewTarget)
{
	if (!NewViewTarget)
	{
		UpdateSpectatorTargetName(TEXT(""));
		return;
	}

	if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(NewViewTarget))
	{
		if (AADPlayerState* PlayerState = Character->GetPlayerState<AADPlayerState>())
		{
			UpdateSpectatorTargetName(PlayerState->GetPlayerNickname());
		}
	}
}
