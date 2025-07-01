#include "Interactable/OtherActors/ADDrone.h"

#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Interactable/OtherActors/Portals/PortalToSubmarine.h"

#include "FrameWork/ADInGameState.h"
#include "Framework/ADGameInstance.h"
#include "Framework/ADPlayerController.h"

#include "Inventory/ADInventoryComponent.h"
#include "ADDroneSeller.h"
#include "Character/UnderwaterCharacter.h"
#include "Gimmic/Spawn/SpawnManager.h"
#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/ADWorldSubsystem.h"
#include "DataRow/PhaseBGMRow.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Framework/ADInGameMode.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"

DEFINE_LOG_CATEGORY(DroneLog);

AADDrone::AADDrone()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	bReplicates = true;
	SetReplicateMovement(true); // 위치 상승하는 것 보이도록
	bIsActive = false;
	bIsFlying = false;
	bIsHold = false;
	ReviveDistance = 1000.f;
}

void AADDrone::BeginPlay()
{
	Super::BeginPlay();
	
	if (!SpawnManager)
	{
		AActor* Found = UGameplayStatics::GetActorOfClass(this, ASpawnManager::StaticClass());
		SpawnManager = Cast<ASpawnManager>(Found);
	
	}
	if (SpawnManager && DronePhaseNumber == 1)
	{
		SpawnManager->SpawnByGroup();
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		DrondeThemeSoundNumber = SoundSubsystem->PlayAttach(ESFX_BGM::DroneTheme, RootComponent);
	}

	WorldSubsystem = GetWorld()->GetSubsystem<UADWorldSubsystem>();
}

void AADDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsActive)
	{
		float DeltaZ = RaiseSpeed * DeltaSeconds;

		FVector Loc = GetActorLocation();
		Loc.Z += DeltaZ;
		SetActorLocation(Loc);

		if (CurrentSeller && IsValid(CurrentSeller))
		{
			FVector SellerLoc = CurrentSeller->GetActorLocation();
			SellerLoc.Z += DeltaZ;
			CurrentSeller->SetActorLocation(SellerLoc);
		}
	}
}

void AADDrone::Destroyed()
{
	SoundSubsystem->StopAudio(DrondeThemeSoundNumber);

	AADPlayerController* PC = GetWorld()->GetFirstPlayerController<AADPlayerController>();
	if (IsValid(PC) == false)
	{
		LOGVN(Error, TEXT("Player Controller is not Valid"));
		Super::Destroyed();
		return;
	}

	UPlayerHUDComponent* PlayerHudComponent = PC->GetPlayerHUDComponent();
	if (PlayerHudComponent == nullptr)
	{
		LOGVN(Error, TEXT("PlayerHudComponent == nullptr"));
		Super::Destroyed();
		return;
	}

	LOGVN(Log, TEXT("OnPhaseChangeDelegate Bound"));
	OnPhaseChangeDelegate.AddUObject(PlayerHudComponent, &UPlayerHUDComponent::PlayNextPhaseAnim);

	LOGV(Log, TEXT("OnPhaseChangeDelegate Broadcast : %d"), DronePhaseNumber + 1);
	OnPhaseChangeDelegate.Broadcast(DronePhaseNumber + 1);
	Super::Destroyed();
}

void AADDrone::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority() || !bIsActive || !IsValid(CurrentSeller) || bIsFlying) return;

	if (!CurrentSeller->GetSubmittedPlayerIndexes().IsEmpty())
	{
		if (AADInGameMode* GameMode = GetWorld()->GetAuthGameMode<AADInGameMode>())
		{
			GameMode->RevivePlayersAroundDroneAtRespawnLocation(CurrentSeller->GetSubmittedPlayerIndexes(), this);
			CurrentSeller->GetSubmittedPlayerIndexes().Empty();
		}
	}
	
	// 차액 계산
	int32 Diff = CurrentSeller->GetCurrentMoney() - CurrentSeller->GetTargetMoney();

	if (Diff > 0)
	{
		if (AADInGameState* GS = GetWorld()->GetGameState<AADInGameState>())
		{
			GS->AddTeamCredit(Diff);
			GS->IncrementPhase();
			if (NextSeller)
			{
				NextSeller->Activate();
				GS->SetCurrentDroneSeller(NextSeller);
				GS->SetDestinationTarget(NextSeller);
			}
			else
			{
				GS->SetCurrentDroneSeller(nullptr);
				GS->SetDestinationTarget(UGameplayStatics::GetActorOfClass(GetWorld(), APortalToSubmarine::StaticClass()));
			}
		}
	}
	CurrentSeller->DisableSelling();
	StartRising();
	bIsFlying = true;
	if (SpawnManager && NextSeller)
	{
		SpawnManager->SpawnByGroup();
		LOGD(Log, TEXT("Monster Spawns"));
	}
	// 다음 BGM 실행
	if (HasAuthority())
	{
		LOGN(TEXT("No PhaseSound, DronePhaseNumber : %d"), DronePhaseNumber);
		LOGN(TEXT("No PhaseSound, DroneName : %s"), *GetName());
		M_PlayPhaseBGM(DronePhaseNumber + 1);
		LOGD(Log,TEXT("Next Phase : PhaseSound"));
	}
}

void AADDrone::M_PlayDroneRisingSound_Implementation()
{
	GetSoundSubsystem()->PlayAt(ESFX::SendDrone, GetActorLocation());
}

// 나중에 수정..
void AADDrone::M_PlayPhaseBGM_Implementation(int32 PhaseNumber)
{
	const FString CurrentMapName = WorldSubsystem ? WorldSubsystem->GetCurrentLevelName() : TEXT("");
	static const FString Context(TEXT("PhaseBGM"));
	if (!PhaseBgmTable)          
	{
		LOGN(TEXT("PhaseBgmTable is nullptr"))
		return;
	}
	const FName RowKey = *FString::Printf(TEXT("%s_%d"), *CurrentMapName, PhaseNumber);

	if (const FPhaseBGMRow* Row = PhaseBgmTable->FindRow<FPhaseBGMRow>(RowKey, Context))
	{
		if (CachedSoundNumber != INDEX_NONE)
			GetSoundSubsystem()->StopAudio(CachedSoundNumber, true);

		CachedSoundNumber = GetSoundSubsystem()->PlayBGM(Row->BGM, true);
		LOGN(TEXT("CurrentMapPhaseBGM : %s"), *RowKey.ToString());
		return;
	}
	LOGN(TEXT("CurrentMapPhaseBGM : %s"), *RowKey.ToString());
	LOGN(TEXT("No BGM row for Map:%s Phase:%d"), *CurrentMapName, PhaseNumber);

	//if (PhaseNumber == 1)
	//{
	//	CachedSoundNumber = GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowPhase1, true);
	//	LOGN(TEXT("PhaseSound1"));
	//}
	//else if (PhaseNumber == 2)
	//{
	//	GetSoundSubsystem()->StopAudio(CachedSoundNumber, true);
	//	CachedSoundNumber = GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowPhase2, true);
	//	LOGN(TEXT("PhaseSound2"));
	//}
	//else if (PhaseNumber == 3)
	//{
	//	GetSoundSubsystem()->StopAudio(CachedSoundNumber, true);
	//	CachedSoundNumber = GetSoundSubsystem()->PlayBGM(ESFX_BGM::ShallowPhase3, true);
	//	LOGN(TEXT("PhaseSound3"));
	//}
	//LOGN(TEXT("No PhaseSound, DronePhaseNumber : %d"), DronePhaseNumber);

}

void AADDrone::Activate()
{
	if (bIsActive) return;
	bIsActive = true;
	OnRep_IsActive(); // 서버에서는 직접 호출해저야함
}

void AADDrone::OnRep_IsActive()
{
	// TODO : UI
}

void AADDrone::StartRising()
{
	SetActorTickEnabled(true);
	GetWorld()->GetTimerManager()
		.SetTimer(DestroyHandle,
			this,
			&AADDrone::OnDestroyTimer,
			DestroyDelay, false
		);
	M_PlayDroneRisingSound();
}

void AADDrone::OnDestroyTimer()
{
	if (CurrentSeller && IsValid(CurrentSeller))
	{
		LOGD(Log, TEXT("[%s] Destroying linked seller %s"), *GetName(), *CurrentSeller->GetName());
		CurrentSeller->Destroy();
	}
	Destroy();
}

void AADDrone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADDrone, bIsActive);
	DOREPLIFETIME(AADDrone, bIsFlying);
}

UADInteractableComponent* AADDrone::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AADDrone::IsHoldMode() const
{
	return bIsHold;
}

FString AADDrone::GetInteractionDescription() const
{
	return TEXT("Send Drone!");
}

const TArray<ATargetPoint*>& AADDrone::GetPlayerRespawnLocations() const
{
	return PlayerRespawnLocations;
}

float AADDrone::GetReviveDistance() const
{
	return ReviveDistance;
}

USoundSubsystem* AADDrone::GetSoundSubsystem()
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

