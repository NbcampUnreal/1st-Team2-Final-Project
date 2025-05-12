#include "Framework/SessionListWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "SessionEntryWidget.h"
#include "ADGameInstance.h"

void USessionListWidget::PopulateSessionList(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (!Scroll_SessionList || !SessionEntryWidgetClass) return;

	Scroll_SessionList->ClearChildren();

	for (const FOnlineSessionSearchResult& Result : SessionResults)
	{
		USessionEntryWidget* Entry = CreateWidget<USessionEntryWidget>(GetWorld(), SessionEntryWidgetClass);
		if (Entry)
		{
			Entry->Setup(Result);
			Scroll_SessionList->AddChild(Entry);
		}
	}
}

void USessionListWidget::NativeConstruct()
{
	Super::NativeConstruct();


	if (Button_Refresh)
	{
		Button_Refresh->OnClicked.AddDynamic(this, &USessionListWidget::OnRefreshClicked);
	}
	if (Button_Back)
	{
		Button_Back->OnClicked.AddDynamic(this, &USessionListWidget::OnBackClicked);
	}
}

void USessionListWidget::OnRefreshClicked()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance()))
	{
		GI->FindSessions();
	}
}

void USessionListWidget::OnBackClicked()
{
	RemoveFromParent(); 
}
