#include "Framework/ADCampGameMode.h"
#include "Framework/ADInGameState.h"
#include "Framework/ADPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AADCampGameMode::ADCampGameMode()
{
	bUseSeamlessTravel = true;
}

void AADCampGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AADPlayerState* ADPlayerState = NewPlayer->GetPlayerState<AADPlayerState>())
	{
		
	}
}

void AADCampGameMode::Logout(AController* Exiting)
{
}

void AADCampGameMode::TryStartGame()
{
	if (AADInGameState* ADGameState = GetGameState<AADInGameState>())
	{
		FString LevelLoad = ADGameState->GetSelectedLevel().ToString();
		if (LevelLoad.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("LevelLoad is empty"));
			LevelLoad = TEXT("DefaultInGameLevel");
			return;
		}
		//input spot level name
		/*FString TravelURL = FString::Printf(TEXT("/Game/Maps/%s?listen"), *LevelLoad);*/
		FString TravelURL = FString::Printf(TEXT("/Game/Maps/%s?listen"), *LevelLoad);
		GetWorld()->ServerTravel(TravelURL);
	}
}

void AADCampGameMode::TravelToInGameLevel()
{
}
