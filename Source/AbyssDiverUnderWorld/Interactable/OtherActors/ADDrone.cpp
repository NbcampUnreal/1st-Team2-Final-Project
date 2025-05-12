#include "Interactable/OtherActors/ADDrone.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "FrameWork/TestGameState.h"
#include "ADDroneSeller.h"

// Sets default values
AADDrone::AADDrone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	bReplicates = true;
}

// Called when the game starts or when spawned
void AADDrone::BeginPlay()
{
	Super::BeginPlay();
	
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

		if (SellerRef && IsValid(SellerRef))
		{
			FVector SellerLoc = SellerRef->GetActorLocation();
			SellerLoc.Z += DeltaZ;
			SellerRef->SetActorLocation(SellerLoc);
		}
	}
}

void AADDrone::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority() || !bIsActive || !SellerRef ||!IsValid(SellerRef)) return;

	// 차액 계산
	int32 Diff = SellerRef->GetCurrentMoney() - SellerRef->GetTargetMoney();
	if (Diff > 0)
	{
		if (ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>())
		{
			GS->AddMoney(Diff);
			GS->IncrementPhase();
		}
	}
	SellerRef->DisableSelling();
	StartRising();
}

void AADDrone::Activate(AADDroneSeller* Seller)
{
	if (!HasAuthority() || bIsActive || !Seller) return;
	bIsActive = true;
	SellerRef = Seller;
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
}

void AADDrone::OnDestroyTimer()
{
	if (SellerRef && IsValid(SellerRef))
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Destroying linked seller %s"), *GetName(), *SellerRef->GetName());
		SellerRef->Destroy();
	}
	Destroy();
}

