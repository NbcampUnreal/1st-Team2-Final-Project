#include "UI/ResultScreen.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"

void UResultScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PlayerNickNameTextArray.IsEmpty() == false)
	{
		return;
	}

	const int PlayerCount = 4;
	FString WidgetNameStr = TEXT("PlayerNickNameText");

	for (int32 i = 1; i <= PlayerCount; ++i)
	{
		FName WidgetName(WidgetNameStr + FString::FromInt(i));
		UTextBlock* TextBlock = CastChecked<UTextBlock>(GetWidgetFromName(WidgetName));
		PlayerNickNameTextArray.Add(TextBlock);
	}

	WidgetNameStr = TEXT("PlayerAliveText");

	for (int32 i = 1; i <= PlayerCount; ++i)
	{
		FName WidgetName(WidgetNameStr + FString::FromInt(i));
		UTextBlock* TextBlock = CastChecked<UTextBlock>(GetWidgetFromName(WidgetName));
		PlayerAliveTextArray.Add(TextBlock);
	}

	WidgetNameStr = TEXT("PlayerContributionText");

	for (int32 i = 1; i <= PlayerCount; ++i)
	{
		FName WidgetName(WidgetNameStr + FString::FromInt(i));
		UTextBlock* TextBlock = CastChecked<UTextBlock>(GetWidgetFromName(WidgetName));
		PlayerContributionTextArray.Add(TextBlock);
	}

	WidgetNameStr = TEXT("PlayerOreText");

	for (int32 i = 1; i <= PlayerCount; ++i)
	{
		FName WidgetName(WidgetNameStr + FString::FromInt(i));
		UTextBlock* TextBlock = CastChecked<UTextBlock>(GetWidgetFromName(WidgetName));
		PlayerOreTextArray.Add(TextBlock);
	}

	check(PlayerNickNameTextArray.IsEmpty() == false);
	check(PlayerAliveTextArray.IsEmpty() == false);
	check(PlayerContributionTextArray.IsEmpty() == false);
	check(PlayerOreTextArray.IsEmpty() == false);
}

void UResultScreen::NativeConstruct()
{
	
}

void UResultScreen::Update(int32 PlayerIndexBased_1, const FResultScreenParams& Params)
{
	ChangePlayerNickNameText(PlayerIndexBased_1, Params.PlayerNickName);
	ChangePlayerAliveText(PlayerIndexBased_1, Params.AliveInfo);
	ChangePlayerContributionText(PlayerIndexBased_1, Params.ContributionScore);
	ChangePlayerOreText(PlayerIndexBased_1, Params.OreScore);
}

void UResultScreen::ChangePlayerNickNameText(int32 PlayerIndexBased_1, const FString& NewText)
{
	PlayerNickNameTextArray[PlayerIndexBased_1 - 1]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangePlayerAliveText(int32 PlayerIndexBased_1, EAliveInfo AliveInfo)
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

	ChangePlayerAliveText(PlayerIndexBased_1, NewText);
}

void UResultScreen::ChangePlayerAliveText(int32 PlayerIndexBased_1, const FString& NewText)
{
	PlayerAliveTextArray[PlayerIndexBased_1 - 1]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangePlayerContributionText(int32 PlayerIndexBased_1, int32 ContributionScore)
{
	ChangePlayerContributionText(PlayerIndexBased_1, FString::FromInt(ContributionScore));
}

void UResultScreen::ChangePlayerContributionText(int32 PlayerIndexBased_1, const FString& NewText)
{
	PlayerContributionTextArray[PlayerIndexBased_1 - 1]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangePlayerOreText(int32 PlayerIndexBased_1, int32 OreScore)
{
	ChangePlayerOreText(PlayerIndexBased_1, FString::FromInt(OreScore));
}

void UResultScreen::ChangePlayerOreText(int32 PlayerIndexBased_1, const FString& NewText)
{
	PlayerOreTextArray[PlayerIndexBased_1 - 1]->SetText(FText::FromString(NewText));
}
