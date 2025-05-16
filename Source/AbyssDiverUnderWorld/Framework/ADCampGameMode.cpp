#include "Framework/ADCampGameMode.h"
#include "Framework/ADInGameState.h"
#include "Framework/ADPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DataRow/PhaseGoalRow.h"

void AADCampGameMode::ADCampGameMode()
{
	bUseSeamlessTravel = true;
}

void AADCampGameMode::SetSelectedLevel(const EMapName InLevelName)
{
	if (AADInGameState* ADGameState = GetGameState<AADInGameState>())
	{
		ADGameState->SetSelectedLevel(InLevelName);
	}
}

void AADCampGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AADPlayerState* ADPlayerState = NewPlayer->GetPlayerState<AADPlayerState>())
	{
		ADPlayerState->ResetLevelResults();
	}
}

void AADCampGameMode::Logout(AController* Exiting)
{
}

void AADCampGameMode::TryStartGame()
{
	TravelToInGameLevel();
}

void AADCampGameMode::TravelToInGameLevel()
{
	if (AADInGameState* ADInGameState = GetGameState<AADInGameState>())
	{
		FString LevelLoad = ADInGameState->GetMapDisplayName();
		if (LevelLoad == "invalid")
		{
			UE_LOG(LogTemp, Error, TEXT("LevelLoad is empty"));
			LevelLoad = TEXT("DefaultInGameLevel");
			return;
		}
		//input spot level name
		FString TravelURL = FString::Printf(TEXT("/Game/_AbyssDiver/Maps/Prototypes_Test/KY/%s?listen"), *LevelLoad);
		GetWorld()->ServerTravel(TravelURL);
	}
}
