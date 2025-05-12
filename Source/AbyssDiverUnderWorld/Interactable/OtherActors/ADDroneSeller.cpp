#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/TestGameState.h"
#include "ADDrone.h"
#include "Net/UnrealNetwork.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"

// Sets default values
AADDroneSeller::AADDroneSeller()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	bReplicates = true;
}

// Called when the game starts or when spawned
void AADDroneSeller::BeginPlay()
{
	Super::BeginPlay();
	
}

void AADDroneSeller::Interact_Implementation(AActor* InstigatorActor)
{
	UE_LOG(LogTemp, Log, TEXT("Not Active"));
	if (!HasAuthority() || !bIsActive) return;
	
	int32 Gained = SellAllExchangeableItems(InstigatorActor);
	if (Gained <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Gained < 0"));
		return;
	}
	

	CurrentMoney += Gained;
	UE_LOG(LogTemp, Log, TEXT("→ 누적 금액: %d / %d"), CurrentMoney, TargetMoney);
	if (CurrentMoney >= TargetMoney && LinkedDrone && LinkedDrone && IsValid(LinkedDrone))
	{
		UE_LOG(LogTemp, Log, TEXT("목표 달성! Drone 활성화 호출"));
		LinkedDrone->Activate(this);
	}
	
}

void AADDroneSeller::DisableSelling()
{
	bIsActive = false;
}

int32 AADDroneSeller::SellAllExchangeableItems(AActor* InstigatorActor)
{
	if (!InstigatorActor) return 0;

	if (APawn* Pawn = Cast<APawn>(InstigatorActor))
	{
		if (AController* C = Pawn->GetController())
		{
			if (UADInventoryComponent* Inv = C->FindComponentByClass<UADInventoryComponent>())
			{
				int32 Price = Inv->GetTotalPrice();
				// TODO : 인벤토리 비우는 함수 구현 필요
				return Price;
			}
		}
	}
	return 0;
}

void AADDroneSeller::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADDroneSeller, bIsActive);
	DOREPLIFETIME(AADDroneSeller, CurrentMoney);
}

