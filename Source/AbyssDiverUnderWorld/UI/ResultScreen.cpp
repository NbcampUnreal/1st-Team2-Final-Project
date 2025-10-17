#include "UI/ResultScreen.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"
#include "UI/ResultScreenSlot.h"

void UResultScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PlayerNickNameTextArray.IsEmpty() == false)
	{
		return;
	}

	const int PlayerCount = 4;

	FString WidgetNameStr = TEXT("PlayerResultInfo");

	for (int32 i = 1; i <= PlayerCount; ++i)
	{
		FName WidgetName(WidgetNameStr + FString::FromInt(i));
		UResultScreenSlot* PlayerInfoSlot = CastChecked<UResultScreenSlot>(GetWidgetFromName(WidgetName));
		PlayerResultInfosArray.Add(PlayerInfoSlot);
	}

	check(PlayerResultInfosArray.IsEmpty() == false);
}

void UResultScreen::Update(int32 PlayerIndex, const FResultScreenParams& Params)
{
	if (PlayerIndex < 0 || PlayerIndex >= PlayerResultInfosArray.Num())
	{
		LOGV(Error, TEXT("인덱스 범위 초과, 범위 : 0~%d, 받은 인덱스 : %d"), PlayerResultInfosArray.Num(), PlayerIndex);
		return;
	}

	PlayerResultInfosArray[PlayerIndex]->SetSlotInfo(Params);
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
