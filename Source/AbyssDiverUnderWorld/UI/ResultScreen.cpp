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

void UResultScreen::Update(int32 PlayerIndex, const FResultScreenParams& Params)
{
	ChangePlayerNickNameText(PlayerIndex, Params.PlayerNickName);
	ChangePlayerAliveText(PlayerIndex, Params.AliveInfo);
	ChangePlayerContributionText(PlayerIndex, Params.ContributionScore);
	ChangePlayerOreText(PlayerIndex, Params.OreScore);
}

void UResultScreen::ChangePlayerNickNameText(int32 PlayerIndex, const FString& NewText)
{
	checkf(PlayerIndex >= 0 && PlayerIndex < PlayerNickNameTextArray.Num(), TEXT("인덱스 범위 초과, 범위 : 0~%d, 받은 인덱스 : %d"), PlayerNickNameTextArray.Num(), PlayerIndex);
	PlayerNickNameTextArray[PlayerIndex]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangePlayerAliveText(int32 PlayerIndex, EAliveInfo AliveInfo)
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

	ChangePlayerAliveText(PlayerIndex, NewText);
}

void UResultScreen::ChangePlayerAliveText(int32 PlayerIndex, const FString& NewText)
{
	checkf(PlayerIndex >= 0 && PlayerIndex < PlayerAliveTextArray.Num(), TEXT("인덱스 범위 초과, 범위 : 0~%d, 받은 인덱스 : %d"), PlayerAliveTextArray.Num(), PlayerIndex);
	PlayerAliveTextArray[PlayerIndex]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangePlayerContributionText(int32 PlayerIndex, int32 ContributionScore)
{
	ChangePlayerContributionText(PlayerIndex, FString::FromInt(ContributionScore));
}

void UResultScreen::ChangePlayerContributionText(int32 PlayerIndex, const FString& NewText)
{
	checkf(PlayerIndex >= 0 && PlayerIndex < PlayerContributionTextArray.Num(), TEXT("인덱스 범위 초과, 범위 : 0~%d, 받은 인덱스 : %d"), PlayerContributionTextArray.Num(), PlayerIndex);
	PlayerContributionTextArray[PlayerIndex]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangePlayerOreText(int32 PlayerIndex, int32 OreScore)
{
	ChangePlayerOreText(PlayerIndex, FString::FromInt(OreScore));
}

void UResultScreen::ChangePlayerOreText(int32 PlayerIndex, const FString& NewText)
{
	checkf(PlayerIndex >= 0 && PlayerIndex < PlayerOreTextArray.Num(), TEXT("인덱스 범위 초과, 범위 : 0~%d, 받은 인덱스 : %d"), PlayerOreTextArray.Num(), PlayerIndex);
	PlayerOreTextArray[PlayerIndex]->SetText(FText::FromString(NewText));
}

void UResultScreen::ChangeTeamMoneyText(int32 NewTeamMoney)
{
	FString NewTeamMoneyText = TEXT("공통 자금 : ") + FString::FromInt(NewTeamMoney);
	ChangeTeamMoneyText(NewTeamMoneyText);
}

void UResultScreen::ChangeTeamMoneyText(const FString& NewText)
{
	TeamMoneyText->SetText(FText::FromString(NewText));
}
