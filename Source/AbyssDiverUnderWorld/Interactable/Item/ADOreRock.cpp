#include "Interactable/Item/ADOreRock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ADItemBase.h"

// Sets default values
AADOreRock::AADOreRock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
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
	if (HasAuthority())
	{
		HandleMineRequest(Cast<APawn>(InstigatorActor));
	}
}
void AADOreRock::HandleMineRequest(APawn* InstigatorPawn)
{
	if (!HasAuthority() || RemainingMines <= 0) return;

	RemainingMines--;
	OnRep_RemainingMines();

	if (RemainingMines <= 0)
	{
		if (FractureFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FractureFX, GetActorLocation());
		}
		if (FractureSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FractureSound, GetActorLocation());
		}

		SpawnDrops();

		Destroy();
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
	for (int32 i = 0; i < Count; i++)
	{
		float Mass = FMath::FRandRange(E->MinMass, E->MaxMass);
		FSoftObjectPath Path = E->ItemClass.ToSoftObjectPath();
		Streamable.RequestAsyncLoad(
			Path,
			FStreamableDelegate::CreateUObject(
				this, &AADOreRock::OnAssetLoaded, E, Mass)
		);
	}
}

void AADOreRock::OnAssetLoaded(FDropEntry* Entry, float Mass)
{
	if (UClass* Class = Entry->ItemClass.Get())
	{
		FActorSpawnParameters Params;
		AADItemBase* Item = GetWorld()->SpawnActor<AADItemBase>(
			Class,
			GetActorLocation(),
			FRotator::ZeroRotator,
			Params
		);
		// TODO
		// Item->SetMass(Mass);
	}
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


