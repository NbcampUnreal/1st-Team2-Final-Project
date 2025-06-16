#include "Framework/ADCampGameMode.h"

#include "Framework/ADInGameState.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADGameInstance.h"
#include "Framework/ADPlayerController.h"

#include "DataRow/PhaseGoalRow.h"
#include "AbyssDiverUnderWorld.h"
#include "Subsystems/DataTableSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

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
	FString NewPlayerId = NewPlayer->GetPlayerState<AADPlayerState>()->GetUniqueId()->ToString();

	LOGV(Warning, TEXT("%s Has Entered"), *NewPlayerId);
	UADGameInstance* GI = GetGameInstance<UADGameInstance>();
	check(GI);

	if (AADPlayerState* ADPlayerState = NewPlayer->GetPlayerState<AADPlayerState>())
	{
		ADPlayerState->ResetLevelResults();
		GI->AddPlayerNetId(NewPlayerId);

		int32 NewPlayerIndex = INDEX_NONE;
		if (GI->TryGetPlayerIndex(NewPlayerId, NewPlayerIndex) == false)
		{
			LOGV(Error, TEXT("Fail To Get Player Index"));
			return;
		}

		ADPlayerState->SetPlayerNickname(NewPlayerId);
		ADPlayerState->SetPlayerIndex(NewPlayerIndex);
	}

	Super::PostLogin(NewPlayer);
}

void AADCampGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	FString ExitingId = Exiting->GetPlayerState<AADPlayerState>()->GetUniqueId().GetUniqueNetId()->ToString();
	UADGameInstance* GI = GetGameInstance<UADGameInstance>();
	GI->RemovePlayerNetId(ExitingId);
}

void AADCampGameMode::TryStartGame()
{
	TravelToInGameLevel();
}

void AADCampGameMode::GetMoney()
{
	AADInGameState* GS = GetGameState<AADInGameState>();
	if (GS == nullptr)
	{
		LOGVN(Error, TEXT("Cheat Failed : GS == nullptr"));
		return;
	}

	GS->SetTotalTeamCredit(GS->GetTotalTeamCredit() + 10000);
}

void AADCampGameMode::TravelToInGameLevel()
{
	if (HasAuthority() == false)
	{
		return;
	}

	for (AADPlayerController* PC : TActorRange<AADPlayerController>(GetWorld()))
	{
		PC->C_OnPreClientTravel();
	}

	FTimerHandle TravelTimerHandle;
	GetWorldTimerManager().SetTimer(TravelTimerHandle, [&]()
		{

			if (AADInGameState* ADInGameState = GetGameState<AADInGameState>())
			{
				EMapName MapName = ADInGameState->GetSelectedLevel();

				FString LevelLoad = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetMapPath(MapName);
				if (LevelLoad == "invalid")
				{
					UE_LOG(LogTemp, Error, TEXT("LevelLoad is empty"));
					return;
				}

				ADInGameState->SendDataToGameInstance();
				//input spot level name
				FString TravelURL = FString::Printf(TEXT("%s?listen"), *LevelLoad);
				GetWorld()->ServerTravel(TravelURL);
			}

		}, 1.0f, false);
}
