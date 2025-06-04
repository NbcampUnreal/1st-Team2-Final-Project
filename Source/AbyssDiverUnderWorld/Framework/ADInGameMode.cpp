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
#include "Projectile/GenericPool.h"
#include "Projectile/ADSpearGunBullet.h"
#include "Subsystems/SoundSubsystem.h"

AADInGameMode::AADInGameMode()
{
	bUseSeamlessTravel = true;

	ConstructorHelpers::FClassFinder<AADSpearGunBullet> SpearGunBulletFinder(TEXT("/Game/_AbyssDiver/Blueprints/Projectile/BP_ADSpearGunBullet"));
	if (SpearGunBulletFinder.Succeeded())
	{
		BulletClass = SpearGunBulletFinder.Class;
	}
}

void AADInGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}
	GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowBackground, 1.0f);
	GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowPhase1, 1.0f);

	if (AADInGameState* InGameState = GetGameState<AADInGameState>())
	{
		UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();

		SpearGunBulletPool = GetWorld()->SpawnActor<AGenericPool>();
		SpearGunBulletPool->InitPool<AADSpearGunBullet>(10, BulletClass);
		LOGVN(Warning, TEXT("SpawnSpearGunBulletPool"));

		int32 LastDroneNumber = 0;
		const int32 FirstDroneNumber = 1;
		for (AADDrone* Drone : TActorRange<AADDrone>(GetWorld()))
		{
			int32 DronePhaseNumber = Drone->GetDronePhaseNumber();
			EMapName MapName = InGameState->GetSelectedLevel();
			FPhaseGoalRow* PhaseGoalRow = DataTableSubsystem->GetPhaseGoalData(MapName, DronePhaseNumber);

			if (ensureMsgf(PhaseGoalRow, TEXT("Map 이름(%d) 또는 DronePhaseNumber(%d)가 유효하지 않습니다. 제대로 설정되어 있는지 확인하세요."), MapName, DronePhaseNumber) == false)
			{
				return;
			}

			check(Drone->CurrentSeller);
			Drone->CurrentSeller->SetTargetMoney(PhaseGoalRow->GoalCredit);

			// 마지막 드론을 기억해서 이 드론이 없으면 게임 클리어 조건 만족으로 사용하려고..
			if (LastDroneNumber < DronePhaseNumber)
			{
				LastDrone = Drone;
				LastDroneNumber = DronePhaseNumber;
			}

			if (DronePhaseNumber == FirstDroneNumber)
			{
				InGameState->SetCurrentDroneSeller(Drone->CurrentSeller);
				Drone->M_PlayPhaseBGM(1);
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
		FString TravelURL = FString::Printf(TEXT("%s?listen"), *LevelLoad);
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
		10000,
		EItemType::Exchangable,
		nullptr
	);

	PS->GetInventory()->AddInventoryItem(ItemData);
}

void AADInGameMode::GetMoney()
{
	AADInGameState* GS = GetGameState<AADInGameState>();
	if (GS == nullptr)
	{
		LOGVN(Error, TEXT("Cheat Failed : GS == nullptr"));
		return;
	}

	GS->SetTotalTeamCredit(GS->GetTotalTeamCredit() + 10000);
}

USoundSubsystem* AADInGameMode::GetSoundSubsystem()
{
	if (SoundSubsystem)
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem;
	}
	return nullptr;
}
