#include "Framework/MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Framework/ADGameInstance.h"

#include "Components/Button.h"

void UMainMenuWidget::NativeConstruct()
{
	if (Button_Create)
		Button_Create->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateClicked);

	if (Button_Join)
		Button_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinClicked);

	if (Button_Options)
		Button_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);

	if (Button_Credits)
		Button_Credits->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);

	if (Button_Quit)
		Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);

}

void UMainMenuWidget::OnCreateClicked()
{
	if (!CreateTeamWidgetInstance && CreateTeamWidgetClass)
	{
		CreateTeamWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), CreateTeamWidgetClass);
	}

	if (CreateTeamWidgetInstance && !CreateTeamWidgetInstance->IsInViewport())
	{
		CreateTeamWidgetInstance->AddToViewport();
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance()))
	{
		GI->FindSessions(); 
	}
}

void UMainMenuWidget::OnOptionsClicked()
{
}

void UMainMenuWidget::OnCreditsClicked()
{
}

void UMainMenuWidget::OnQuitClicked()
{
}
