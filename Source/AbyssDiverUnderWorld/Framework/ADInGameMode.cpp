#include "Framework/ADInGameMode.h"

#include "AbyssDiverUnderWorld.h"
#include "ADInGameState.h"
#include "ADPlayerState.h"
#include "ADPlayerController.h"

#include "Interactable/OtherActors/ADDrone.h"
#include "Interactable/OtherActors/ADDroneSeller.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/DataTableSubsystem.h"

#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Projectile/GenericPool.h"
#include "Projectile/ADSpearGunBullet.h"

#include "DataRow/PhaseGoalRow.h"
#include "Container/FStructContainer.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

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
				InGameState->SetDestinationTarget(Drone->CurrentSeller);
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

	if (ensure(Exiting) == false)
	{
		return;
	}

	AADPlayerState* PS = Exiting->GetPlayerState<AADPlayerState>();
	if (ensure(PS) == false)
	{
		return;
	}

	const FUniqueNetIdRepl& UniqueNetIdRepl = PS->GetUniqueId();
	if (ensure(&UniqueNetIdRepl) == false)
	{
		return;
	}

	FString ExitingId = UniqueNetIdRepl->ToString();
	UADGameInstance* GI = GetGameInstance<UADGameInstance>();
	GI->RemovePlayerNetId(ExitingId);

	LOGVN(Error, TEXT("Logout, Who : %s, NetId : %s"), *Exiting->GetName(), *ExitingId);

}

void AADInGameMode::ReadyForTravelToCamp()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	if (TimerManager.IsTimerActive(ResultTimerHandle))
	{
		return;
	}

	LOGVN(Warning, TEXT("Ready For Traveling to Camp..."));

	for (AADPlayerController* PC : TActorRange<AADPlayerController>(GetWorld()))
	{
		PC->GetPlayerHUDComponent()->C_ShowResultScreen();
	}

	TimerManager.ClearTimer(ResultTimerHandle);
	
	const float Interval = 5.0f;
	TimerManager.SetTimer(ResultTimerHandle, this, &AADInGameMode::TravelToCamp, 1, false, Interval);
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

void AADInGameMode::BindDelegate(AUnderwaterCharacter* PlayerCharacter)
{
	if (PlayerCharacter == nullptr)
	{
		LOGV(Error, TEXT("PlayerCharacter == nullptr"));
		return;
	}

	if (PlayerCharacter->OnCharacterStateChangedDelegate.IsAlreadyBound(this, &AADInGameMode::OnCharacterStateChanged))
	{
		return;
	}

	LOGV(Error, TEXT("DelegateBound"));
	PlayerCharacter->OnCharacterStateChangedDelegate.AddDynamic(this, &AADInGameMode::OnCharacterStateChanged);
}

void AADInGameMode::InitPlayer(APlayerController* PC)
{
	if (!PC)
		return;

	if (!PC->GetPawn())
	{
		RestartPlayer(PC);
	}
}

void AADInGameMode::GameOver()
{
	LOGV(Error, TEXT("Game Over"));

#if WITH_EDITOR
	// PIE 버그로 이걸 호출해주지 않으면 Server Travel시 크래시 일어남.
	if (GEditor)
	{
		GEditor->SelectNone(true, true);
	}

#endif

	ReadyForTravelToCamp();
}

void AADInGameMode::OnCharacterStateChanged(ECharacterState OldCharacterState, ECharacterState NewCharacterState)
{
	LOGV(Error, TEXT("Begin, NewCharacterState : %d, PlayerNum : %d"), NewCharacterState, GetNumPlayers());
	if (NewCharacterState == ECharacterState::Death)
	{
		if (OldCharacterState == ECharacterState::Groggy)
		{
			GroggyCount--;
		}

		DeathCount++;
	}
	else if (NewCharacterState == ECharacterState::Groggy)
	{
		LOGV(Error, TEXT("Groggy"));
		GroggyCount++;
	}
	else if (OldCharacterState == ECharacterState::Groggy && NewCharacterState == ECharacterState::Normal)
	{
		GroggyCount--;
	}

	if (DeathCount + GroggyCount == GetNumPlayers())
	{
		GameOver();
	}

	LOGV(Error, TEXT("End"));
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
