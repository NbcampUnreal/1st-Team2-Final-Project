#include "Interactable/OtherActors/ADDrone.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "FrameWork/ADInGameState.h"
#include "ADDroneSeller.h"
#include "Net/UnrealNetwork.h"
#include "Gimmic/Spawn/SpawnManager.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/SoundSubsystem.h"

DEFINE_LOG_CATEGORY(DroneLog);

// Sets default values
AADDrone::AADDrone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	bReplicates = true;
	SetReplicateMovement(true); // 위치 상승하는 것 보이도록
	bIsActive = false;

	bIsHold = false;
}

// Called when the game starts or when spawned
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

void AADDrone::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority() || !bIsActive || !IsValid(CurrentSeller)) return;

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
			}
			else
			{
				GS->SetCurrentDroneSeller(nullptr);
			}
		}
	}
	CurrentSeller->DisableSelling();
	StartRising();
	if (SpawnManager && NextSeller)
	{
		SpawnManager->SpawnByGroup();
		LOGD(Log, TEXT("Monster Spawns"));
	}
}

void AADDrone::M_PlayDroneRisingSound_Implementation()
{
	GetSoundSubsystem()->PlayAt(ESFX::SendDrone, GetActorLocation());
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
