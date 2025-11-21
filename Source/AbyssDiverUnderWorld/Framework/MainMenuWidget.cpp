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

	if (Button_Tutorial)
		Button_Tutorial->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTutorialClicked);

	if (Button_Options)
		Button_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);

	if (Button_Credits)
		Button_Credits->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);

	if (Button_Quit)
		Button_Quit->OnClicked.AddDynamic(this, &	UMainMenuWidget::OnQuitClicked);

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{ 
		SetPreTutorialButtonsEnabled(GI->bHasPlayedTutorial);
	} 
}

void UMainMenuWidget::OnCreateClicked()
{
	if (!CreateTeamWidgetInstance && CreateTeamWidgetClass)
	{
		CreateTeamWidgetInstance = CreateWidget<UCreateTeamWidget>(GetWorld(), CreateTeamWidgetClass);
		CreateTeamWidgetInstance->OnBackClickedDelegate.BindUObject(this, &UMainMenuWidget::OnCreateTeamWidgetClosed);
	}
	else if (CreateTeamWidgetInstance)
	{
		CreateTeamWidgetInstance->StopAllAnimations();
		CreateTeamWidgetInstance->RemoveFromParent();
		CreateTeamWidgetInstance = nullptr;
	}

	if (CreateTeamWidgetInstance && !CreateTeamWidgetInstance->IsInViewport())
	{
		CreateTeamWidgetInstance->AddToViewport();
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	
}

void UMainMenuWidget::OnTutorialClicked()
{
	UGameplayStatics::OpenLevel(this, FName("TutorialPool"));
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
	if (CreateTeamWidgetInstance)
	{
		CreateTeamWidgetInstance->StopAllAnimations();
		CreateTeamWidgetInstance->RemoveFromParent();
		CreateTeamWidgetInstance = nullptr; 
	}


}

void UMainMenuWidget::OnSessionListWidgetClosed()
{
	if (SessionListWidgetInstance)
	{
		CreateTeamWidgetInstance->StopAllAnimations();
		CreateTeamWidgetInstance->RemoveFromParent();
		CreateTeamWidgetInstance = nullptr;
	}
}

void UMainMenuWidget::SetPreTutorialButtonsEnabled(bool bEnable)
{
	if (bEnable)
	{
		Button_Create->SetIsEnabled(true);
		Button_Join->SetIsEnabled(true);
	}
	else
	{
		Button_Create->SetIsEnabled(false);
		Button_Join->SetIsEnabled(false);
	}
}
