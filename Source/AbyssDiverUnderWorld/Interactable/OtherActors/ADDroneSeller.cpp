#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADInGameState.h"
#include "ADDrone.h"
#include "Net/UnrealNetwork.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Framework/ADPlayerState.h"

// Sets default values
AADDroneSeller::AADDroneSeller()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	bReplicates = true;
	SetReplicateMovement(true); // 위치 상승하는 것 보이도록

	bIsActive = true;
	bIsHold = false;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AADDroneSeller::BeginPlay()
{
	Super::BeginPlay();
	
}

void AADDroneSeller::Interact_Implementation(AActor* InstigatorActor)
{
	LOGD(Log, TEXT("Not Active"));
	if (!HasAuthority() || !bIsActive) return;
	
	int32 Gained = SellAllExchangeableItems(InstigatorActor);
	if (Gained <= 0)
	{
		LOGD(Log, TEXT("Gained < 0"))
		return;
	}
	
	SetCurrentMoeny(CurrentMoney + Gained);
	//CurrentMoney += Gained;
	LOGD(Log, TEXT("→ 누적 금액: %d / %d"), CurrentMoney, TargetMoney);
	if (CurrentMoney >= TargetMoney && IsValid(CurrentDrone))
	{
		LOGD(Log, TEXT("목표 달성! Drone 활성화 호출"))
		CurrentDrone->Activate();
	}
	
}

void AADDroneSeller::DisableSelling()
{
	bIsActive = false;
}

void AADDroneSeller::Activate()
{
	if (bIsActive) return;

	bIsActive = true;
	OnRep_IsActive(); // 서버에서는 직접 호출해저야함
}


void AADDroneSeller::OnRep_IsActive()
{
	//TODO : UI 업데이트
}

void AADDroneSeller::OnRep_CurrentMoney()
{
	OnCurrentMoneyChangedDelegate.Broadcast(CurrentMoney);
}

void AADDroneSeller::OnRep_TargetMoney()
{
	OnTargetMoneyChangedDelegate.Broadcast(TargetMoney);
}

int32 AADDroneSeller::SellAllExchangeableItems(AActor* InstigatorActor)
{
	if (!InstigatorActor) return 0;

	if (APawn* Pawn = Cast<APawn>(InstigatorActor))
	{
		if (AController* C = Pawn->GetController())
		{
			if (AADPlayerState* PS = Cast<AADPlayerState>(C->PlayerState))
			{
				if (UADInventoryComponent* Inv = PS->GetInventory())
				{
					int32 Price = Inv->GetTotalPrice();
					// TODO : 인벤토리 비우는 함수 구현 필요
					TArray<int8> TypeArray = Inv->GetInventoryIndexesByType(EItemType::Exchangable);
					for (int i = 0; i<TypeArray.Num(); ++i)
					{
						if (TypeArray[i] > -1)
						{
							Inv->RemoveBySlotIndex(i, EItemType::Exchangable, false);
						}
					}
					return Price;
				}
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
	DOREPLIFETIME(AADDroneSeller, TargetMoney);
}

UADInteractableComponent* AADDroneSeller::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AADDroneSeller::IsHoldMode() const
{
	return bIsHold;
}

FString AADDroneSeller::GetInteractionDescription() const
{
	return TEXT("Submit Ore!");
}
