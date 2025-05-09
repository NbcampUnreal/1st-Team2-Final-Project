#include "Interactable/Item/ADOreRock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ADItemBase.h"
#include "ADExchangeableItem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraSystem.h"  
#include "NiagaraFunctionLibrary.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"

// Sets default values
AADOreRock::AADOreRock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// InteractableComponent 생성
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
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

void AADOreRock::Interact(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	HandleMineRequest(Cast<APawn>(InstigatorActor));
}
void AADOreRock::HandleMineRequest(APawn* InstigatorPawn)
{
	if (!HasAuthority() || RemainingMines <= 0) return;

	RemainingMines--;
	OnRep_RemainingMines();

	if (RemainingMines > 0)
	{
		if (PickAxeImpactFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				PickAxeImpactFX,
				GetActorLocation(),
				FRotator::ZeroRotator
			);
		}
	}

	if (RemainingMines <= 0)
	{
		if (RockFragmentsFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				RockFragmentsFX,
				GetActorLocation(),
				FRotator::ZeroRotator
			);
		}

		if (FractureSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FractureSound, GetActorLocation());
		}

		SpawnDrops();

		//Destroy();
	}
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
		float Mass = SampleDropMass(E->MinMass, E->MaxMass);
		FSoftObjectPath Path = E->ItemClass.ToSoftObjectPath();
		UAssetManager& AssetMgr = UAssetManager::Get();
		AssetMgr.GetStreamableManager().RequestAsyncLoad(
			Path,
			FStreamableDelegate::CreateUObject(this, &AADOreRock::OnAssetLoaded, E, Mass)
		);
	}
}

void AADOreRock::OnAssetLoaded(FDropEntry* Entry, float Mass)
{
	if (UClass* Class = Entry->ItemClass.Get())
	{
		const float SpawnHeight = 150.f;  // 스폰 높이
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

float AADOreRock::SampleDropMass(float MinMass, float MaxMass) const
{
	// 균등 난수 생성
	float u = FMath::FRand();

	// pow(u, α) → 저울질 편향
	float biased = FMath::Pow(u, MassBiasExponent);

	// 편향된 값을 [MinMass, MaxMass] 구간에 선형 매핑
	return FMath::Lerp(MinMass, MaxMass, biased);
}

void AADOreRock::OnRep_RemainingMines()
{
	// TODO: UI update
}

void AADOreRock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADOreRock, RemainingMines);
}


