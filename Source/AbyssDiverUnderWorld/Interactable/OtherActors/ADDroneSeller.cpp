#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADInGameState.h"
#include "ADDrone.h"
#include "Net/UnrealNetwork.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/MissionSubsystem.h"

AADDroneSeller::AADDroneSeller()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	bReplicates = true;
	SetReplicateMovement(true); // 위치 상승하는 것 보이도록

	bIsActive = true;
	bIsHold = false;
	bAlwaysRelevant = true;
}

void AADDroneSeller::BeginPlay()
{
	Super::BeginPlay();
	
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}

	if (IsValid(GetMissionSubsystem()) == false)
	{
		LOGV(Error, TEXT("IsValid(GetMissionSubsystem()) == false"));
		return;
	}

	MissionSubsystem->RequestBinding(this);
}

void AADDroneSeller::Destroyed()
{
#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	if (IsValid(GetMissionSubsystem()) == false)
	{
		LOGV(Error, TEXT("IsValid(GetMissionSubsystem()) == false"));
		return;
	}

	MissionSubsystem->RequestUnbinding(this);

	Super::Destroyed();
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
	LOGD(Log, TEXT("→ 누적 금액: %d / %d"), CurrentMoney, TargetMoney);
	if (CurrentMoney >= TargetMoney && IsValid(CurrentDrone))
	{
		LOGD(Log, TEXT("목표 달성! Drone 활성화 호출"))
		CurrentDrone->Activate();
		GetSoundSubsystem()->PlayAt(ESFX::ActivateDrone, GetActorLocation());
	}
	else
	{
		GetSoundSubsystem()->PlayAt(ESFX::SubmitOre, GetActorLocation());
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
	if (CurrentMoney >= TargetMoney)
	{
		GetSoundSubsystem()->PlayAt(ESFX::ActivateDrone, GetActorLocation());
	}
	else
	{
		GetSoundSubsystem()->PlayAt(ESFX::SubmitOre, GetActorLocation());
	}
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
					TArray<int8> TypeArray = Inv->GetInventoryIndexesByType(EItemType::Exchangable);
					
					for (int i = 0; i < TypeArray.Num(); ++i)
					{
						if (TypeArray[i] > -1)
						{
							uint8 OreId = Inv->GetInventoryList().Items[i].Id;
							int32 OreMass = Inv->GetInventoryList().Items[i].Mass;

							Inv->RemoveBySlotIndex(i, EItemType::Exchangable, false);
							OnSellOreDelegate.Broadcast(OreId, OreMass);
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

USoundSubsystem* AADDroneSeller::GetSoundSubsystem()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}

	return SoundSubsystem;
}

UMissionSubsystem* AADDroneSeller::GetMissionSubsystem()
{
	MissionSubsystem = GetGameInstance()->GetSubsystem<UMissionSubsystem>();
	return MissionSubsystem;
}
