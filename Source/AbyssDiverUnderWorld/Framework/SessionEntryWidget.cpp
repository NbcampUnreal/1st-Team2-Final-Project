#include "Framework/SessionEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ADGameInstance.h"

void USessionEntryWidget::Setup(const FOnlineSessionSearchResult& InResult)
{
	SearchResult = InResult;

	FString ServerName;
	if (SearchResult.Session.SessionSettings.Get(FName("ServerName"), ServerName))
	{
		if (Text_SessionName)
			Text_SessionName->SetText(FText::FromString(ServerName));
	}
	else
	{
		Text_SessionName->SetText(FText::FromString("Unnamed Session"));
	}

	int8 Current = SearchResult.Session.SessionSettings.NumPublicConnections
					- SearchResult.Session.NumOpenPublicConnections;
	int8 Max = SearchResult.Session.SessionSettings.NumPublicConnections;

	if (Text_PlayerCount)
	{
		Text_PlayerCount->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Current, Max)));
	}
}

void USessionEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Join)
	{
		Button_Join->OnClicked.AddDynamic(this, &USessionEntryWidget::OnJoinClicked);
	}
}

void USessionEntryWidget::OnJoinClicked()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance()))
	{
		GI->JoinSession(SearchResult);
	}
}
