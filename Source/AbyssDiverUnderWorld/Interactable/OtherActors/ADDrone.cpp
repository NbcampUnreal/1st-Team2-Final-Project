#include "Interactable/OtherActors/ADDrone.h"

#include "Inventory/ADInventoryComponent.h"
#include "ADDroneSeller.h"
#include "Gimmic/Spawn/SpawnManager.h"
#include "DataRow/PhaseBGMRow.h"

#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Interactable/OtherActors/Portals/PortalToSubmarine.h"

#include "FrameWork/ADInGameState.h"
#include "Framework/ADGameInstance.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADInGameMode.h"

#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/ADWorldSubsystem.h"
#include "Subsystems/Localizations/LocalizationSubsystem.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "UI/InteractPopupWidget.h"

#include "Framework/ADTutorialGameMode.h"   
#include "Framework/ADTutorialGameState.h"

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
	bIsBgmOn = true;
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

	WorldSubsystemWeakPtr = GetWorld()->GetSubsystem<UADWorldSubsystem>();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystemWeakPtr = GI->GetSubsystem<USoundSubsystem>();
		
		FTimerHandle PlayBGMDelayTimerHandle;
		float PlayBGMDelay = 2.0f;
		GetWorldTimerManager().SetTimer(PlayBGMDelayTimerHandle, [this]() {
			if (bIsBgmOn)
			{
				const FString CurrentMapName = GetWorldSubsystem() ? GetWorldSubsystem()->GetCurrentLevelName() : TEXT("");
				if (CurrentMapName != "TutorialPool" && SoundSubsystemWeakPtr.IsValid())
				{
					DroneThemeAudioId = SoundSubsystemWeakPtr->PlayAttach(ESFX_BGM::DroneTheme, RootComponent);
				}
			}}, PlayBGMDelay, false);
	}
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
#if WITH_EDITOR

// 게임 중이 아닌 경우 리턴
UWorld* World = GetWorld();
if (World == nullptr || World->IsGameWorld() == false)
{
	Super::Destroyed();
	return;
}

#endif
	if (bIsBgmOn)
	{
		if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
		{
			SoundSubsystem->StopAudio(DroneThemeAudioId);
			SoundSubsystem->StopAudio(TutorialAlarmSoundId);
		}
	}

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
	UE_LOG(LogTemp, Warning, TEXT("ADDrone::Interact_Implementation -- 함수가 성공적으로 호출됨!"));
	if (AADTutorialGameMode* TutorialMode = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADDrone - 튜토리얼 게임 모드를 찾음. 부활 시퀀스 호출!"));
		if (TutorialMode)
		{
			if (AADTutorialGameState* TutorialGS = TutorialMode->GetGameState<AADTutorialGameState>())
			{
				if (TutorialGS->GetCurrentPhase() == ETutorialPhase::Step15_Resurrection)
				{
					TutorialMode->TriggerResurrectionSequence();
				}
				if (TutorialGS->GetCurrentPhase() == ETutorialPhase::Step7_Drone)
				{
					TutorialMode->AdvanceTutorialPhase();
				}
			}
		}
		StartRising();
		return;
	}

	if (!HasAuthority() || !bIsActive || !IsValid(CurrentSeller) || bIsFlying) return;

	APlayerController* PC = Cast<APlayerController>(InstigatorActor->GetInstigatorController());
	if(!PC) return;

	if (AADPlayerController* PlayerController = InstigatorActor->GetInstigatorController<AADPlayerController>())
	{
		if (UPlayerHUDComponent* PlayerHUDComponent = PlayerController->GetPlayerHUDComponent())
		{
			PlayerHUDComponent->C_ShowConfirmWidget(this);
		}
	}
}

void AADDrone::M_PlayTutorialAlarmSound_Implementation()
{
	TutorialAlarmSoundId = GetSoundSubsystem()->PlayAttach(ESFX_BGM::DroneTutorialAlarm, RootComponent);
}

void AADDrone::M_PlayDroneRisingSound_Implementation()
{
	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->PlayAt(ESFX::SendDrone, GetActorLocation());
	}
}

// 나중에 수정..
void AADDrone::M_PlayPhaseBGM_Implementation(int32 PhaseNumber)
{
	USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (SoundSubsystem == nullptr)
		return;
	
	ESFX_BGM BGM = GetPhaseBGM(PhaseNumber);
	if (BGM != ESFX_BGM::Max)
	{
		if (BGMAudioID != INDEX_NONE)
			SoundSubsystem->StopAudio(BGMAudioID, true);

		BGMAudioID = SoundSubsystem->PlayBGM(BGM, true);
		LOGN(TEXT("PhaseBGM Played : %d"), (int32)BGM);
	}

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

ESFX_BGM AADDrone::GetPhaseBGM(int32 PhaseNumber) const
{
	if (!PhaseBgmTable)          
	{
		LOGN(TEXT("PhaseBgmTable is nullptr"))
		return ESFX_BGM::Max;
	}
	
	const FString CurrentMapName = GetWorldSubsystem() ? GetWorldSubsystem()->GetCurrentLevelName() : TEXT("");
	static const FString Context(TEXT("PhaseBGM"));
	const FName RowKey = *FString::Printf(TEXT("%s_%d"), *CurrentMapName, PhaseNumber);
	if (const FPhaseBGMRow* Row = PhaseBgmTable->FindRow<FPhaseBGMRow>(RowKey, Context))
	{
		LOGN(TEXT("CurrentMapPhaseBGM : %s"), *RowKey.ToString());
		return Row->BGM;
	}
	
	LOGN(TEXT("No BGM row for Map:%s Phase:%d"), *CurrentMapName, PhaseNumber);
	return ESFX_BGM::Max;
}

void AADDrone::ExecuteConfirmedInteraction()
{
	if (AADTutorialGameMode* TutorialGameMode = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
	{
		UE_LOG(LogTemp, Log, TEXT("Drone Interaction: Tutorial Mode Detected. Calling TriggerResurrectionSequence."));

		TutorialGameMode->TriggerResurrectionSequence();

		return;
	}

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
	int32 ExtraMoney = CurrentSeller->GetCurrentMoney() - CurrentSeller->GetTargetMoney();
	if (ExtraMoney > 0)
	{
		if (AADInGameState* GS = GetWorld()->GetGameState<AADInGameState>())
		{
			GS->AddTeamCredit(ExtraMoney);
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
	M_PlayPhaseBGM(DronePhaseNumber + 1);
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
	ULocalizationSubsystem* LocalizationSubsystem = GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
	if (IsValid(LocalizationSubsystem) == false)
	{
		LOGV(Error, TEXT("Cant Get LocalizationSubsystem"));
		return "";
	}

	return LocalizationSubsystem->GetLocalizedText(ST_InteractionDescription::TableKey, ST_InteractionDescription::Drone_SendDrone).ToString();
}

const TArray<ATargetPoint*>& AADDrone::GetPlayerRespawnLocations() const
{
	return PlayerRespawnLocations;
}

float AADDrone::GetReviveDistance() const
{
	return ReviveDistance;
}

USoundSubsystem* AADDrone::GetSoundSubsystem() const
{
	if (!SoundSubsystemWeakPtr.IsValid())
	{
		if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
		{
			SoundSubsystemWeakPtr = GI->GetSubsystem<USoundSubsystem>();
		}
	}
	
	return SoundSubsystemWeakPtr.Get();
}

UADWorldSubsystem* AADDrone::GetWorldSubsystem() const
{
	if (!WorldSubsystemWeakPtr.IsValid())
	{
		WorldSubsystemWeakPtr = GetWorld() != nullptr ? GetWorld()->GetSubsystem<UADWorldSubsystem>() : nullptr;
	}

	return WorldSubsystemWeakPtr.Get();
}

