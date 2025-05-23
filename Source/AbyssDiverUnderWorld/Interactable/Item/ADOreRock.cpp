#include "Interactable/Item/ADOreRock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ADItemBase.h"
#include "ADExchangeableItem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraSystem.h"  
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"
#include "Character/UnderwaterCharacter.h"

// Sets default values
AADOreRock::AADOreRock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// InteractableComponent 생성
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsHold = true;
	bMiningActive = false;

	SpawnedTool = nullptr;
	ActiveMiningVFX = nullptr;
	ActiveMiningSFX = nullptr;
}

// Called when the game starts or when spawned
void AADOreRock::BeginPlay()
{
	Super::BeginPlay();
	if (!DropTable) return;

	static const FString Ctxt;
	for (FName& Name : DropTable->GetRowNames())
	{
		if (FDropEntry* E = DropTable->FindRow<FDropEntry>(Name, Ctxt))
		{
			CachedEntries.Add(E);
			TotalWeight += E->SpawnWeight;
			CumulativeWeights.Add(TotalWeight);
		}
	}
}

void AADOreRock::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	HandleMineRequest(Cast<APawn>(InstigatorActor));
}

void AADOreRock::InteractHold_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	HandleMineRequest(Cast<APawn>(InstigatorActor));
}

void AADOreRock::OnHoldStart_Implementation(APawn* InstigatorPawn)
{
	LOGI(Log, TEXT("Mining Starts"));
	SpawnAndAttachTool(InstigatorPawn);
	PlayMiningAnim(InstigatorPawn);
}

void AADOreRock::OnHoldStop_Implementation(APawn* InstigatorPawn)
{
	LOGI(Log, TEXT("Mining Stops"));
	PlayStowAnim(InstigatorPawn);
	CleanupToolAndEffects();
}

void AADOreRock::HandleMineRequest(APawn* InstigatorPawn)
{
	if (!HasAuthority() || CurrentMiningGauge <= 0) return;

	if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InstigatorPawn))
	{
		CurrentMiningGauge = FMath::Max(0, CurrentMiningGauge - DefaultMiningStrength * UnderwaterCharacter->GetGatherMultiplier());
		OnRep_CurrentMiningGauge();
		PlayMiningFX();

		// 채광 이펙트
		if (CurrentMiningGauge > 0 && PickAxeImpactFX)
		{
			FVector MiningLocation = GetActorLocation() + (0, 0, 90);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), PickAxeImpactFX, MiningLocation, FRotator::ZeroRotator
			);
		}

		// 게이지 소진 시 파괴 이펙트 & 드롭
		if (CurrentMiningGauge <= 0)
		{
			PlayFractureFX();
			SpawnDrops();
			//Destroy();
		}
	}
	PlayStowAnim(InstigatorPawn);
	CleanupToolAndEffects();
}

void AADOreRock::SpawnDrops()
{
	if (CachedEntries.Num() == 0 || TotalWeight <= 0.f) return;

	float R = FMath::FRandRange(0.f, TotalWeight);
	int16 Index = Algo::LowerBound(CumulativeWeights, R);
	if (!CachedEntries.IsValidIndex(Index)) return;
	FDropEntry* E = CachedEntries[Index];

	int8 Count = FMath::RandRange(E->MinCount, E->MaxCount);
	for (int8 i = 0; i < Count; i++)
	{
		int32 Mass = SampleDropMass(E->MinMass, E->MaxMass);
		FSoftObjectPath Path = E->ItemClass.ToSoftObjectPath();
		UAssetManager& AssetMgr = UAssetManager::Get();
		AssetMgr.GetStreamableManager().RequestAsyncLoad(
			Path,
			FStreamableDelegate::CreateUObject(this, &AADOreRock::OnAssetLoaded, E, Mass)
		);
	}
}

void AADOreRock::OnAssetLoaded(FDropEntry* Entry, int32 Mass)
{
	if (UClass* Class = Entry->ItemClass.Get())
	{
		FVector SpawnLoc = GetActorLocation() + FVector(0, 0, SpawnHeight);

		FActorSpawnParameters Params;
		AADItemBase* Item = GetWorld()->SpawnActor<AADItemBase>(
			Class,
			SpawnLoc,
			FRotator::ZeroRotator,
			Params
		);
		Item->SetItemMass(Mass);

		// 스폰 이후 발사체 컴포넌트 활성화
		if (AADExchangeableItem* ExItem = Cast<AADExchangeableItem>(Item))
		{
			ExItem->CalculateTotalPrice();

			FVector RandomXY = FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), 0);
			FVector DropDir = RandomXY + FVector(0, 0, -200); // 아래쪽으로 힘
			ExItem->DropMovement->Velocity = DropDir;
			ExItem->DropMovement->Activate();
		}
		

		// TODO
		// Item->SetMass(Mass);
	}

	Destroy();
}

void AADOreRock::PlayMiningFX()
{
	if (PickAxeImpactFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), PickAxeImpactFX, GetActorLocation());
}

void AADOreRock::PlayFractureFX()
{
	if (RockFragmentsFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), RockFragmentsFX, GetActorLocation());

	if (FractureSound)
		UGameplayStatics::PlaySoundAtLocation(
			this, FractureSound, GetActorLocation());
}

int32 AADOreRock::SampleDropMass(int32 MinMass, int32 MaxMass) const
{
	// 균등 난수 생성
	float u = FMath::FRand();

	// pow(u, α) → 저울질 편향
	float biased = FMath::Pow(u, MassBiasExponent);

	// 편향된 값을 [MinMass, MaxMass] 구간에 선형 매핑
	float FValue = FMath::Lerp((float)MinMass, (float)MaxMass, biased);

	return FMath::RoundToInt(FValue);
}

void AADOreRock::OnRep_CurrentMiningGauge()
{
	// TODO: UI update
	PlayMiningFX();

	if (CurrentMiningGauge == 0)
		PlayFractureFX();
}

void AADOreRock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADOreRock, CurrentMiningGauge);
	DOREPLIFETIME(AADOreRock, bMiningActive);
}

void AADOreRock::SpawnAndAttachTool(APawn* InstigatorPawn)
{
	if (SpawnedTool || !MiningToolClass || !InstigatorPawn) return;

	FActorSpawnParameters Params;
	Params.Owner = InstigatorPawn;
	Params.Instigator = InstigatorPawn;

	SpawnedTool = GetWorld()->SpawnActor<AActor>(
		MiningToolClass,
		InstigatorPawn->GetActorLocation(),
		InstigatorPawn->GetActorRotation(),
		Params
	);

	if (SpawnedTool)
	{

		SpawnedTool->SetActorEnableCollision(false);

		if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InstigatorPawn))
		{
			SpawnedTool->AttachToComponent(
				UnderwaterCharacter->GetMesh1P(),
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				TEXT("Laser")
			);
			SpawnedTool->AttachToComponent(
				UnderwaterCharacter->GetMesh(),
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				TEXT("Laser")
			);
		}
	}
}

void AADOreRock::PlayMiningAnim(APawn* InstigatorPawn)
{
	if (!MiningMontage || !InstigatorPawn) return;

	if (AUnderwaterCharacter* UC = Cast<AUnderwaterCharacter>(InstigatorPawn))
	{
		// 1) 채집에 맞는 SyncState 구성
		FAnimSyncState MiningSync;
		MiningSync.bEnableRightHandIK = true;  
		MiningSync.bEnableLeftHandIK = false;
		MiningSync.bEnableFootIK = true;
		MiningSync.bIsStrafing = false;

		// 2) 1P & 3P 동시에 재생
		UC->M_PlayMontageOnBothMesh(
			MiningMontage,
			1.0f,
			NAME_None,
			MiningSync
		);
	}
}

void AADOreRock::PlayStowAnim(APawn* InstigatorPawn)
{
	if (!StowMontage || !InstigatorPawn) return;

	if (AUnderwaterCharacter* UC = Cast<AUnderwaterCharacter>(InstigatorPawn))
	{
		FAnimSyncState StowSync;                // 대부분 IK 끄고 기본 Strafe 복귀
		StowSync.bEnableRightHandIK = false;
		StowSync.bEnableLeftHandIK = false;
		StowSync.bEnableFootIK = true;
		StowSync.bIsStrafing = false;

		UC->M_PlayMontageOnBothMesh(
			StowMontage,
			1.0f,
			NAME_None,
			StowSync
		);
	}
}

void AADOreRock::CleanupToolAndEffects()
{
	if (SpawnedTool)
	{
		SpawnedTool->Destroy();
		SpawnedTool = nullptr;
	}
}

UADInteractableComponent* AADOreRock::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AADOreRock::IsHoldMode() const
{
	return bIsHold;
}

float AADOreRock::GetHoldDuration_Implementation() const
{
	return HoldDuration;
}

