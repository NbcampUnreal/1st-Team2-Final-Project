#include "Framework/ADInGameMode.h"
#include "ADInGameState.h"
#include "ADPlayerState.h"
#include "ADPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Framework/ADGameInstance.h"
#include "Interactable/OtherActors/ADDrone.h"
#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/PhaseGoalRow.h"
#include "AbyssDiverUnderWorld.h"
#include "Container/FStructContainer.h"
#include "Inventory/ADInventoryComponent.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AADInGameMode::AADInGameMode()
{
	bUseSeamlessTravel = true;
}

void AADInGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (AADInGameState* InGameState = GetGameState<AADInGameState>())
	{
		UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();

		int32 LastDroneNumber = 0;

		for (AADDrone* Drone : TActorRange<AADDrone>(GetWorld()))
		{
			int32 DronePhaseNumber = Drone->GetDronePhaseNumber();
			EMapName MapName = InGameState->GetSelectedLevel();
			FPhaseGoalRow* PhaseGoalRow = DataTableSubsystem->GetPhaseGoalData(MapName, DronePhaseNumber);
			check(PhaseGoalRow)
			check(Drone->CurrentSeller);
			Drone->CurrentSeller->SetTargetMoney(PhaseGoalRow->GoalCredit);

			// 마지막 드론을 기억해서 이 드론이 없으면 게임 클리어 조건 만족으로 사용하려고..
			if (LastDroneNumber < DronePhaseNumber)
			{
				LastDrone = Drone;
				LastDroneNumber = DronePhaseNumber;
			}
		}
	}
}

void AADInGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	InitPlayer(NewPlayer);
}

void AADInGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	FString ExitingId = Exiting->GetPlayerState<AADPlayerState>()->GetUniqueId().GetUniqueNetId()->ToString();
	UADGameInstance* GI = GetGameInstance<UADGameInstance>();
	GI->RemovePlayerNetId(ExitingId);

}

void AADInGameMode::ReadyForTravelToCamp()
{
	LOGVN(Warning, TEXT("Ready For Traveling to Camp..."));

	for (AADPlayerController* PC : TActorRange<AADPlayerController>(GetWorld()))
	{
		PC->GetPlayerHUDComponent()->C_ShowResultScreen();
	}

	FTimerHandle TimerHandle;
	const float Interval = 5.0f;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AADInGameMode::TravelToCamp, 1, false, Interval);
}

void AADInGameMode::TravelToCamp()
{
	if (AADInGameState* ADInGameState = GetGameState<AADInGameState>())
	{
		FString LevelLoad = CampMapName;
		if (LevelLoad == "invalid")
		{
			UE_LOG(LogTemp, Error, TEXT("LevelLoad is empty"));
			LevelLoad = TEXT("DefaultInGameLevel");
			return;
		}

		ADInGameState->SendDataToGameInstance();
		//input spot level name
		FString TravelURL = FString::Printf(TEXT("/Game/_AbyssDiver/Maps/Prototypes_Test/KY/%s?listen"), *LevelLoad);
		GetWorld()->ServerTravel(TravelURL);
	}
}

bool AADInGameMode::IsAllPhaseCleared()
{
	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS == nullptr)
	{
		LOGVN(Warning, TEXT("GS == nullptr"));
		return false;
	}

	// 현재 패이즈가 최대랑 같고 마지막 드론이 Destroy된 상태라면 true
	return GS->GetPhase() == GS->GetMaxPhase() && (::IsValid(LastDrone) == false);
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

void AADInGameMode::GetOre()
{
	AADPlayerState* PS = Cast<AADPlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), 0));
	if (PS == nullptr)
	{
		LOGVN(Error, TEXT("Cheat FAiled : PS == nullptr"));
		return;
	}

	FItemData ItemData
	(
		FName(TEXT("CobaltShell")),
		8,
		1,
		99,
		1,
		1,
		1000,
		EItemType::Exchangable,
		nullptr
	);

	PS->GetInventory()->AddInventoryItem(ItemData);
}
