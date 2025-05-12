#include "Framework/MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Framework/ADGameInstance.h"
#include "Blueprint/WidgetTree.h"

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
		CreateTeamWidgetInstance = CreateWidget<UCreateTeamWidget>(GetWorld(), CreateTeamWidgetClass);
		CreateTeamWidgetInstance->OnBackClickedDelegate.BindUObject(this, &UMainMenuWidget::OnCreateTeamWidgetClosed);
	}

	if (CreateTeamWidgetInstance && !CreateTeamWidgetInstance->IsInViewport())
	{
		CreateTeamWidgetInstance->AddToViewport();

		TArray<UWidget*> Widgets;
		WidgetTree->GetAllWidgets(Widgets);

		for (UWidget* Widget : Widgets)
		{
			if (Widget)
			{
				Widget->SetIsEnabled(false);
			}
		}
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	//replace blueprint 
}

void UMainMenuWidget::OnOptionsClicked()
{
}

void UMainMenuWidget::OnCreditsClicked()
{
}

void UMainMenuWidget::OnQuitClicked()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, true);
	}
}

void UMainMenuWidget::OnCreateTeamWidgetClosed()
{
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);

	for (UWidget* Widget : Widgets)
	{
		if (Widget)
		{
			Widget->SetIsEnabled(true);
		}
	}
}
