#include "Framework/ADInGameMode.h"
#include "ADInGameState.h"
#include "ADPlayerState.h"
#include "ADPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AADInGameMode::AADInGameMode()
{
}

void AADInGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (AADInGameState* InGameState = GetGameState<AADInGameState>())
	{
		
	}

}

void AADInGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	InitPlayer(NewPlayer);
}

void AADInGameMode::InitPlayer(APlayerController* PC)
{
	if (!PC)
		return;

	if (!PC->GetPawn())
	{
		RestartPlayer(PC);
	}

	if (AADPlayerState* ADPlayerState = Cast<AADPlayerState>(PC->PlayerState))
	{
		ADPlayerState->ResetLevelResults();

	}
}
