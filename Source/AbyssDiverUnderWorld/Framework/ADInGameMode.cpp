#include "Framework/ADInGameMode.h"

#include "AbyssDiverUnderWorld.h"
#include "ADInGameState.h"
#include "ADPlayerState.h"
#include "ADPlayerController.h"

#include "Interactable/OtherActors/ADDrone.h"
#include "Interactable/OtherActors/ADDroneSeller.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/MissionSubsystem.h"

#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Projectile/GenericPool.h"
#include "Projectile/ADSpearGunBullet.h"
#include "Projectile/ADFlareGunBullet.h"

#include "DataRow/PhaseGoalRow.h"
#include "Container/FStructContainer.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Algo/RandomShuffle.h"
#include "Engine/TargetPoint.h"

AADInGameMode::AADInGameMode()
{
	bUseSeamlessTravel = true;

	ConstructorHelpers::FClassFinder<AADSpearGunBullet> SpearGunBulletFinder(TEXT("/Game/_AbyssDiver/Blueprints/Projectile/BP_ADSpearGunBullet"));
	if (SpearGunBulletFinder.Succeeded())
	{
		SpearBulletClass = SpearGunBulletFinder.Class;
	}
	ConstructorHelpers::FClassFinder<AADFlareGunBullet> FlareGunBulletFinder(TEXT("/Game/_AbyssDiver/Blueprints/Projectile/BP_ADFlareGunBullet"));
	if (FlareGunBulletFinder.Succeeded())
	{
		FlareBulletClass = FlareGunBulletFinder.Class;
	}
}

void AADInGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

#if !WITH_EDITOR
	ErrorMessage = TEXT("게임 도중 입장 불가");
#endif
}

void AADInGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}

	if (IsValid(SoundSubsystem) == false)
	{
		GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowBackground, 1.0f);
		GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowPhase1, 1.0f);
	}

	if (AADInGameState* InGameState = GetGameState<AADInGameState>())
	{
		UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();

		SpearGunBulletPool = GetWorld()->SpawnActor<AGenericPool>();
		SpearGunBulletPool->InitPool<AADSpearGunBullet>(30, SpearBulletClass);
		FlareGunBulletPool = GetWorld()->SpawnActor<AGenericPool>();
		FlareGunBulletPool->InitPool<AADFlareGunBullet>(30, FlareBulletClass);
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
	/*			Drone->M_PlayPhaseBGM(1);*/
			}
		}
	}

	PlayerAliveInfos.Init(EPlayerAliveInfo::Alive, GetNumPlayers());
}

void AADInGameMode::StartPlay()
{
	Super::StartPlay();           // 모든 Actor BeginPlay 이후 호출

	for (AADDrone* Drone : TActorRange<AADDrone>(GetWorld()))
	{
		if (Drone->GetDronePhaseNumber() == 1)
		{
			Drone->M_PlayPhaseBGM(1);
		}
	}
}

void AADInGameMode::PostLogin(APlayerController* NewPlayer)
{
	FString NewPlayerId = NewPlayer->GetPlayerState<AADPlayerState>()->GetUniqueId()->ToString();

	LOGV(Warning, TEXT("%s Has Entered"), *NewPlayerId);
	UADGameInstance* GI = GetGameInstance<UADGameInstance>();
	check(GI);

	UMissionSubsystem* MissionSubsystem = GI->GetSubsystem<UMissionSubsystem>();
	if (MissionSubsystem == nullptr)
	{
		LOGV(Error, TEXT("Fail to get MissionSubsystem"));
		return;
	}

	MissionSubsystem->RemoveAllMissions();

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

		ADPlayerState->SetPlayerIndex(NewPlayerIndex);
	}

	Super::PostLogin(NewPlayer);

	InitPlayer(NewPlayer);
}

void AADInGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (IsValid(Exiting) == false)
	{
		LOGV(Error, TEXT("IsValid(Exiting) == false"));
		return;
	}

	AADPlayerState* PS = Exiting->GetPlayerState<AADPlayerState>();
	if (IsValid(PS) == false)
	{
		LOGV(Error, TEXT("IsValid(PS) == false"));
		return;
	}

	const FUniqueNetIdRepl& UniqueNetIdRepl = PS->GetUniqueId();
	if (UniqueNetIdRepl.IsValid() == false)
	{
		LOGV(Error, TEXT("UniqueNetIdRepl.IsValid() == false"));
		return;
	}

	FString ExitingId = UniqueNetIdRepl->ToString();
	UADGameInstance* GI = GetGameInstance<UADGameInstance>();
	GI->RemovePlayerNetId(ExitingId);

	UMissionSubsystem* MissionSubsystem = GI->GetSubsystem<UMissionSubsystem>();
	if (MissionSubsystem == nullptr)
	{
		LOGV(Error, TEXT("Fail to get MissionSubsystem"));
		return;
	}

	MissionSubsystem->RemoveAllMissions();


	int32 LogoutPlayerIndex = PS->GetPlayerIndex();
	if (PlayerAliveInfos.IsValidIndex(LogoutPlayerIndex) == false)
	{
		LOGV(Error, TEXT("Not Valid Player Index"));
		return;
	}

	PlayerAliveInfos[LogoutPlayerIndex] = EPlayerAliveInfo::Absent;
	
	LOGVN(Error, TEXT("Logout, Who : %s, NetId : %s"), *Exiting->GetName(), *ExitingId);

}

void AADInGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	if (AADPlayerState* PlayerState = NewPlayer->GetPlayerState<AADPlayerState>())
	{
		PlayerState->SetHasBeenDead(false);
		
		int32 PlayerIndex = PlayerState->GetPlayerIndex();

		if (PlayerAliveInfos.IsValidIndex(PlayerIndex) == false)
		{
			LOGVN(Error, TEXT("Not Valid Index : %d, ArrayNum : %d"), PlayerIndex, PlayerAliveInfos.Num());
			return;
		}

		LOGVN(Log, TEXT("Player Restarted, PlayerIndex : %d"), PlayerIndex);
	}
}

void AADInGameMode::ReadyForTravelToCamp()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	if (TimerManager.IsTimerActive(ResultTimerHandle))
	{
		return;
	}

	for (AADPlayerState* ADPlayerState : TActorRange<AADPlayerState>(GetWorld()))
	{
		APawn* Player = ADPlayerState->GetPawn();
		if (Player == nullptr || IsValid(Player) == false || Player->IsValidLowLevel() == false || Player->IsPendingKillPending())
		{
			LOGV(Error, TEXT("Not Valid Player, PlayeStateName : %s"), *ADPlayerState->GetName());
			continue;
		}

		ADPlayerState->GetPawn()->bAlwaysRelevant = true;
	}

	ForceNetUpdate();

	LOGV(Log, TEXT("Releveant On"));

	TimerManager.ClearTimer(SyncTimerHandle);
	const float WaitForSync = 1.0f;
	TimerManager.SetTimer(SyncTimerHandle, [&]()
		{
			
			LOGVN(Log, TEXT("Ready For Traveling to Camp..."));

			UWorld* World = GetWorld();
			if (IsValid(World) == false || World->IsInSeamlessTravel() || World->IsValidLowLevel() == false || World->bIsTearingDown)
			{
				LOGV(Error, TEXT("World Is Not Valid"));
				return;
			}

			for (AADPlayerController* PC : TActorRange<AADPlayerController>(World))
			{
				PC->GetPlayerHUDComponent()->C_ShowResultScreen();
			}

		}, WaitForSync, false);

	TimerManager.ClearTimer(ResultTimerHandle);
	const float Interval = 9.0f;
	TimerManager.SetTimer(ResultTimerHandle, this, &AADInGameMode::TravelToCamp, 1, false, Interval);
}

void AADInGameMode::TravelToCamp()
{
	for (AADPlayerController* PC : TActorRange<AADPlayerController>(GetWorld()))
	{
		PC->C_OnPreClientTravel();
	}

	UMissionSubsystem* MissionSubsystem = GetGameInstance()->GetSubsystem<UMissionSubsystem>();
	if (MissionSubsystem == nullptr)
	{
		LOGV(Error, TEXT("Fail to get MissionSubsystem"));
		return;
	}

	MissionSubsystem->RemoveAllMissions();
	const float WaitForStopVoice = 1.0f;

	FTimerHandle WaitForVoiceTimerHandle;
	GetWorldTimerManager().SetTimer(WaitForVoiceTimerHandle, [&]()
		{
			if (AADInGameState* ADInGameState = GetGameState<AADInGameState>())
			{
				FString LevelLoad = CampMapName;
				if (LevelLoad == "invalid")
				{
					LOGV(Error, TEXT("LevelLoad is empty"));
					return;
				}

				ADInGameState->SendDataToGameInstance();
				//input spot level name
				FString TravelURL = FString::Printf(TEXT("%s?listen"), *LevelLoad);
				GetWorld()->ServerTravel(TravelURL);
			}

		}, WaitForStopVoice, false);
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

void AADInGameMode::RevivePlayersAtRandomLocation(TArray<int8> PlayerIndexes, const FVector& SpawnCenter,
	const float ReviveDistance)
{
	for (int8 PlayerIndex : PlayerIndexes)
	{
		FVector RandomLocation = GetRandomLocation(SpawnCenter, ReviveDistance);
		RestartPlayerFromPlayerIndex(PlayerIndex, RandomLocation);
	}
}

void AADInGameMode::RevivePlayersAroundDroneAtRespawnLocation(const TArray<int8>& PlayerIndexes, const AADDrone* SomeDrone)
{
	if (IsValid(SomeDrone) == false)
	{
		LOGV(Error, TEXT("Drone Is Invalid"));
		return;
	}

	const TArray<ATargetPoint*>& RespawnLocationCandidates = SomeDrone->GetPlayerRespawnLocations();

	const int32 CandidateCount = RespawnLocationCandidates.Num();
	if (CandidateCount == 0)
	{
		RevivePlayersAtRandomLocation(PlayerIndexes, SomeDrone->GetActorLocation(), SomeDrone->GetReviveDistance());
		return;
	}

	TArray<int32> SelectedIndexes;
	SelectedIndexes.Reserve(CandidateCount);
	for (int32 i = 0; i < CandidateCount; ++i)
	{
		SelectedIndexes.Add(i);
	}

	const int32 PlayerIndexCount = PlayerIndexes.Num();

	int32 CurrentCandidateIndex = CandidateCount;
	for (int32 i = 0; i < PlayerIndexCount; ++i)
	{
		if (CurrentCandidateIndex >= CandidateCount)
		{
			if (CandidateCount > 1)
			{
				Algo::RandomShuffle(SelectedIndexes);
			}

			CurrentCandidateIndex = 0;
		}

		int32 SelectedIndex = SelectedIndexes[CurrentCandidateIndex];
		const FVector RespawnLocation = RespawnLocationCandidates[SelectedIndex]->GetActorLocation();
		
		int32 PlayerIndex = PlayerIndexes[i];

		RestartPlayerFromPlayerIndex(PlayerIndex, RespawnLocation);
		CurrentCandidateIndex++;
	}
}

AADPlayerController* AADInGameMode::FindPlayerControllerFromIndex(int8 PlayerIndex) const
{
	for (AADPlayerController* PlayerController : TActorRange<AADPlayerController>(GetWorld()))
	{
		AADPlayerState* PlayerState = Cast<AADPlayerState>(PlayerController->PlayerState);
		if (PlayerState && PlayerState->GetPlayerIndex() == PlayerIndex)
		{
			return PlayerController;
		}
	}

	return nullptr;
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

FVector AADInGameMode::GetRandomLocation(const FVector& Location, float Distance) const
{
	float CapsuleRadius = 34.0f;
	float CapsuleHalfHeight = 88.0f;
	if (AUnderwaterCharacter* DefaultCharacter = DefaultPawnClass->GetDefaultObject<AUnderwaterCharacter>())
	{
		CapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		CapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	
	FVector RandomLocation = FVector::ZeroVector;
	int MaxAttempts = 10;
	while (MaxAttempts > 0)
	{
		RandomLocation = Location + FMath::VRand() * Distance;
		MaxAttempts--;

		FCollisionQueryParams Params;
		Params.bTraceComplex = true;
		FHitResult HitResult;
		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult,
			RandomLocation,
			RandomLocation,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
			Params
		);

		if (!bHit)
		{
			return RandomLocation;
		}
	}

	// 랜덤한 위치를 찾지 못했을 경우 어쨌든 위치를 반환
	UE_LOG(AbyssDiver, Warning, TEXT("Failed to find a random location after multiple attempts. Returning last calculated location."));
	return RandomLocation;
}

void AADInGameMode::RestartPlayerFromPlayerIndex(int8 PlayerIndex, const FVector& SpawnLocation)
{
	AADPlayerController* PlayerController = FindPlayerControllerFromIndex(PlayerIndex);
	// 현재 관전이 없으므로 Hide 되어 있다.
	// if (PlayerController == nullptr || PlayerController->GetPawn() != nullptr)
	if (PlayerController == nullptr)
	{
		return;
	}

	if (APawn* PlayerPawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		PlayerPawn->Destroy();
	}

	FRotator SpawnRotator = FRotator::ZeroRotator;
	RestartPlayerAtTransform(PlayerController, FTransform(SpawnRotator, SpawnLocation));
}

void AADInGameMode::GameOver()
{
	LOGV(Log, TEXT("Game Over"));

#if WITH_EDITOR
	// PIE 버그로 이걸 호출해주지 않으면 Server Travel시 크래시 일어남.
	if (GEditor)
	{
		GEditor->SelectNone(true, true);
	}

#endif

	ReadyForTravelToCamp();
	
	for (AADPlayerController* PC : TActorRange<AADPlayerController>(GetWorld()))
	{
		PC->C_PlayGameOverSound();
	}
}

void AADInGameMode::OnCharacterStateChanged(AUnderwaterCharacter* Character, ECharacterState OldCharacterState, ECharacterState NewCharacterState)
{
	AADPlayerState* PS = Character->GetPlayerState<AADPlayerState>();
	if (PS == nullptr)
	{
		LOGV(Error, TEXT("Not Valid Player State"));
		return;
	}

	int32 PlayerIndex = PS->GetPlayerIndex();

	if (PlayerAliveInfos.IsValidIndex(PlayerIndex) == false)
	{
		LOGV(Error, TEXT(" Not Valid Index From PlayerAliveInfos, Total : %d, Index : %d"), PlayerAliveInfos.Num(), PlayerIndex);
		return;
	}

	if (PlayerAliveInfos[PlayerIndex] == EPlayerAliveInfo::Absent)
	{
		return;
	}

	PlayerAliveInfos[PlayerIndex] = (NewCharacterState == ECharacterState::Normal) ? EPlayerAliveInfo::Alive : EPlayerAliveInfo::Dead;

	LOGV(Log, TEXT("Begin, Old State : %d,  NewCharacterState : %d, PlayerNum : %d, PlayerIndex : %d"), OldCharacterState, NewCharacterState, GetNumPlayers(), PlayerIndex);

	bool bIsGameOver = true;
	for (const EPlayerAliveInfo& AliveInfo : PlayerAliveInfos)
	{
		if (AliveInfo == EPlayerAliveInfo::Alive)
		{
			bIsGameOver = false;
			break;
		}
	}

	if (bIsGameOver)
	{
		GameOver();
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
		0,
		0,
		10000,
		EItemType::Exchangable,
		EBulletType::None,
		nullptr
	);

	PS->GetInventory()->AddInventoryItem(ItemData);
}

void AADInGameMode::GetMoney()
{
	GetSomeMoney(10000);
}

void AADInGameMode::GetSomeMoney(int32 SomeValue)
{
	AADInGameState* GS = GetGameState<AADInGameState>();
	if (GS == nullptr)
	{
		LOGVN(Error, TEXT("Cheat Failed : GS == nullptr"));
		return;
	}

	GS->SetTotalTeamCredit(GS->GetTotalTeamCredit() + SomeValue);
}

USoundSubsystem* AADInGameMode::GetSoundSubsystem()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}

	return SoundSubsystem;
}
