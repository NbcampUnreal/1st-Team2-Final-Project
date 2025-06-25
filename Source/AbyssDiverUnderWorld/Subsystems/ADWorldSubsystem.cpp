#include "Subsystems/ADWorldSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerController.h"

const FString UADWorldSubsystem::MainMenuLevelName = TEXT("MainLevel");
const FString UADWorldSubsystem::CampLevelName = TEXT("Submarine_Lobby");

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

	AADPlayerController* PC = InWorld.GetFirstPlayerController <AADPlayerController>();
	if (PC == nullptr)
	{
		return;
	}

	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
	PC->SetIgnoreMoveInput(false);
	// 카메라 페이드 아웃이 적용되어 있으면 원래대로 복구한다.
	PC->ShowFadeIn();
}
