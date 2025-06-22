#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADInGameState.h"
#include "ADDrone.h"
#include "Character/UnderwaterCharacter.h"
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

	// ✅ StaticMeshComponent를 가져와서 캐싱 (머테리얼 교체용)
	CachedMesh = FindComponentByClass<UStaticMeshComponent>();

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

	SubmitPlayer(InstigatorActor);
	
	int32 Gained = SellAllExchangeableItems(InstigatorActor);
	if (Gained <= 0)
	{
		LOGD(Log, TEXT("Gained < 0"));
		return;
	}

	SetCurrentMoeny(CurrentMoney + Gained);

	LOGD(Log, TEXT("→ 누적 금액: %d / %d"), CurrentMoney, TargetMoney);

	// 🔸 모든 클라이언트에서 색상 전환 연출
	const bool bReachedGoal = (CurrentMoney >= TargetMoney);
	Multicast_TemporarilyHighlightGreen(bReachedGoal);

	if (bReachedGoal && IsValid(CurrentDrone))
	{
		LOGD(Log, TEXT("목표 달성! Drone 활성화 호출"));
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
	OnMoneyRatioChangedDelegate.Broadcast(MoneyRatio);

	bool bReachedGoal = (CurrentMoney >= TargetMoney);

	GetSoundSubsystem()->PlayAt(
		bReachedGoal ? ESFX::ActivateDrone : ESFX::SubmitOre,
		GetActorLocation()
	);

	// ✅ 여기서도 색상 설정
	SetLightColor(bReachedGoal ? FLinearColor::Green : FLinearColor::Red);
}

void AADDroneSeller::OnRep_TargetMoney()
{
	OnTargetMoneyChangedDelegate.Broadcast(TargetMoney);
}

void AADDroneSeller::SubmitPlayer(AActor* InstigatorActor)
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(InstigatorActor);
	if (Player == nullptr)
	{
		return;
	}

	for (AUnderwaterCharacter* BoundCharacter : Player->GetBoundCharacters())
	{
		if (!IsValid(BoundCharacter) || BoundCharacter->IsPendingKillPending())
		{
			continue;
		}

		int8 PlayerIndex = BoundCharacter->GetPlayerIndex();
		if (SubmittedPlayerIndexes.Contains(PlayerIndex))
		{
			LOGD(Log, TEXT("이미 제출된 플레이어: %s"), *BoundCharacter->GetName());
			continue;
		}

		LOGD(Log, TEXT("제출된 플레이어: %s"), *BoundCharacter->GetName());
		SubmittedPlayerIndexes.Add(PlayerIndex);

		BoundCharacter->UnBind();

		// @ToDo: 추후에 Destroy 호출
		// 원래라면 Destroy를 호출해야 하지만, 현재 관전이 없기 때문에 일단은 숨기기만 한다. 
		BoundCharacter->SetActorHiddenInGame(true);
	}
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
					const TArray<FItemData>& Items = Inv->GetInventoryList().Items;

					int32 ItemCount = Items.Num();
					for (int32 i = 0; i < ItemCount; ++i)
					{
						int32 Index = ItemCount - i - 1;
						if (Items[Index].ItemType == EItemType::Exchangable)
						{
							int32 SlotIndex = Items[Index].SlotIndex;

							uint8 OreId = Items[Index].Id;
							int32 OreMass = Items[Index].Mass;

							Inv->RemoveBySlotIndex(SlotIndex, EItemType::Exchangable, false);
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
	DOREPLIFETIME(AADDroneSeller, MoneyRatio);
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

void AADDroneSeller::SetLightColor(FLinearColor NewColor)
{
	if (!IsValid(CachedMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ CachedMesh is not valid"));
		return;
	}

	UMaterialInterface* DesiredMaterial = nullptr;

	if (NewColor == FLinearColor::Red)
	{
		DesiredMaterial = RedMaterial;
	}
	else if (NewColor == FLinearColor::Green)
	{
		DesiredMaterial = GreenMaterial;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ Unsupported color"));
		return;
	}

	if (DesiredMaterial)
	{
		CachedMesh->SetMaterial(0, DesiredMaterial);
		UE_LOG(LogTemp, Warning, TEXT("✅ Set material to %s"), *DesiredMaterial->GetName());
	}
}

void AADDroneSeller::Multicast_TemporarilyHighlightGreen_Implementation(bool bReachedGoal)
{
	SetLightColor(FLinearColor::Green);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this, bReachedGoal]()
		{
			SetLightColor(bReachedGoal ? FLinearColor::Green : FLinearColor::Red);
		},
		0.5f,
		false
	);
}
