#include "Subsystems/ADWorldSubsystem.h"

#include "AbyssDiverUnderWorld.h"

const FString UADWorldSubsystem::MainMenuLevelName = TEXT("MainLevel");

void UADWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	FString WorldName;
	InWorld.GetName(WorldName);
	LOGV(Log, TEXT("%s Map Has BegunPlay"), *WorldName);

	if (MainMenuLevelName == WorldName)
	{
		return;
	}

	APlayerController* PC = InWorld.GetFirstPlayerController <APlayerController>();
	if (PC == nullptr)
	{
		return;
	}

	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
	PC->SetIgnoreMoveInput(false);
}
