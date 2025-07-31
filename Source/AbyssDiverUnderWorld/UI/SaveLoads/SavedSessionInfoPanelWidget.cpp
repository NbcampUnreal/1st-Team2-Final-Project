#include "UI/SaveLoads/SavedSessionInfoPanelWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"

const int32 USavedSessionInfoPanelWidget::MaxPlayerNumber = 4;

void USavedSessionInfoPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerNameTextArray.Reset(MaxPlayerNumber);

	static const FString PlayerNameTextPrefix = TEXT("PlayerNameText_");

	for (int32 i = 0; i < MaxPlayerNumber; ++i)
	{
		FName PlayerNameText(FString::Printf(TEXT("%s%s"), *PlayerNameTextPrefix, *FString::FromInt(i)));

		UTextBlock* PlayerNameTextInstance = Cast<UTextBlock>(GetWidgetFromName(PlayerNameText));
		check(PlayerNameTextInstance);
		PlayerNameTextArray.Emplace(PlayerNameTextInstance);
	}
}

void USavedSessionInfoPanelWidget::SetSavedNameText(const FString& NewText)
{
	if (IsValid(SaveNameText) == false)
	{
		LOGV(Error, TEXT("SaveNameText is not valid"));
		return;
	}

	SaveNameText->SetText(FText::FromString(NewText));
}

void USavedSessionInfoPanelWidget::SetSaveDateText(const FString& NewText)
{
	if (IsValid(SaveDateText) == false)
	{
		LOGV(Error, TEXT("SaveDateText is not valid"));
		return;
	}

	SaveDateText->SetText(FText::FromString(NewText));
}

void USavedSessionInfoPanelWidget::SetClearCountNumberText(int32 NewClearCountNum)
{
	if (IsValid(ClearCountNumberText) == false)
	{
		LOGV(Error, TEXT("ClearCountText is not valid"));
		return;
	}

	ClearCountNumberText->SetText(FText::FromString(FString::FromInt(NewClearCountNum)));
}

void USavedSessionInfoPanelWidget::SetPlayerNameTexts(const TArray<FString>& PlayerNameTexts)
{
	const int32 PlayerNameTextCount = FMath::Min(PlayerNameTexts.Num(), MaxPlayerNumber);

	for (int32 i = 0; i < PlayerNameTextCount; ++i)
	{
		PlayerNameTextArray[i]->SetText(FText::FromString(PlayerNameTexts[i]));
	}

	for (int32 i = PlayerNameTextCount; i < MaxPlayerNumber; ++i)
	{
		PlayerNameTextArray[i]->SetText(FText());
	}
}