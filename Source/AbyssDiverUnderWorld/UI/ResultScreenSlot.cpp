// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ResultScreenSlot.h"
#include "AbyssDiverUnderWorld.h"
#include "Components/TextBlock.h"

void UResultScreenSlot::SetSlotInfo(const FResultScreenParams& Params)
{
	ChangePlayerNickNameText(Params.PlayerNickName);
	ChangePlayerAliveText(Params.AliveInfo);
	ChangePlayerCollectionText(FString::FromInt(Params.CollectionScore));
	ChangePlayerBattleText(FString::FromInt(Params.BattleScore));
	ChangePlayerSupportText(FString::FromInt(Params.SupportScore));
}

void UResultScreenSlot::ChangePlayerNickNameText(const FString& NewText)
{
	PlayerNickNameText->SetText(FText::FromString(NewText));
}

void UResultScreenSlot::ChangePlayerAliveText(EAliveInfo AliveInfo)
{
	if (AliveInfo >= EAliveInfo::MAX)
	{
		LOGV(Error, TEXT("AliveInfo >= EAliveInfo::MAX"));
		return;
	}

	FString NewText = "";

	switch (AliveInfo)
	{
	case EAliveInfo::Alive:
		NewText = TEXT("Alive");
		break;
	case EAliveInfo::Abandoned:
		NewText = TEXT("Abandoned");
		break;
	case EAliveInfo::Dead:
		NewText = TEXT("Dead");
		break;
	default:
		check(false);
		return;
	}

	PlayerAliveText->SetText(FText::FromString(NewText));
}

void UResultScreenSlot::ChangePlayerCollectionText(const FString& NewText)
{
	PlayerCollectionText->SetText(FText::FromString(NewText));
}

void UResultScreenSlot::ChangePlayerBattleText(const FString& NewText)
{
	PlayerBattleText->SetText(FText::FromString(NewText));
}

void UResultScreenSlot::ChangePlayerSupportText(const FString& NewText)
{
	PlayerSupportText->SetText(FText::FromString(NewText));
}
