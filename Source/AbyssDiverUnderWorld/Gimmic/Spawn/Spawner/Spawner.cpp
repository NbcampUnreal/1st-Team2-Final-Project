#include "Gimmic/Spawn/Spawner/Spawner.h"

#include "AbyssDiverUnderWorld.h"
#include "Gimmic/Spawn/SpawnPoint/SpawnPoint.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"

ASpawner::ASpawner()
{
	LocationRange = CreateDefaultSubobject<UBoxComponent>(TEXT("LocationRange"));
	LocationRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	MinDistance = 100.0f;
	MaxTries = 100;
}

void ASpawner::BeginPlay()
{
	Super::BeginPlay();
}

ASpawnPoint* ASpawner::GetPossibleSpawnPoint()
{
	TArray<ASpawnPoint*> SpawnPointArray = TotalSpawnPoints.Array();

	if (SpawnPointArray.Num() == 0)
	{
		return nullptr;
	}

	// 최대 MaxTries 만큼 Spawn 지점 탐색
	for (int32 Try = 0; Try < MaxTries; ++Try)
	{
		ASpawnPoint* Candidate = SpawnPointArray[FMath::RandRange(0, SpawnPointArray.Num() - 1)];
		if (!IsValid(Candidate)) continue;
		
		for (ASpawnPoint* UsedPoint : SpawnSuccessPoints)
		{
			if (!IsValid(UsedPoint)) continue;

			const float Distance = FVector::Dist(UsedPoint->GetActorLocation(), Candidate->GetActorLocation());
			if (Distance >= MinDistance)
			{
				RemoveTotalSpawnPoint(Candidate);
				AddSpawnSuccessPoint(Candidate);
				return Candidate;
			}
		}
	}

	// 조건을 만족하는 포인트를 찾지 못한 경우: 그냥 랜덤한 포인트 반환
	ASpawnPoint* FallbackPoint = SpawnPointArray[FMath::RandRange(0, SpawnPointArray.Num() - 1)];
	RemoveTotalSpawnPoint(FallbackPoint);
	AddSpawnSuccessPoint(FallbackPoint);
	return FallbackPoint;
}

void ASpawner::RemoveAllSpawnPoints()
{
	for (ASpawnPoint* SpawnPoint : TotalSpawnPoints)
	{
		if (IsValid(SpawnPoint))
		{
			SpawnPoint->Destroy();
		}
	}

	TotalSpawnPoints.Empty();
	SpawnSuccessPoints.Empty();
}

TSet<TObjectPtr<ASpawnPoint>> ASpawner::GetSpawnPoint(TSubclassOf<ASpawnPoint> SpawnPointClass)
{
	if (!IsValid(SpawnPointClass)|| !IsValid(LocationRange)) return TSet<TObjectPtr<ASpawnPoint>>();

	const FBox SpawnBounds = LocationRange->CalcBounds(LocationRange->GetComponentTransform()).GetBox();

	TSet<TObjectPtr<ASpawnPoint>> SpawnPointArray;

	for (TActorIterator<ASpawnPoint> It(GetWorld(), SpawnPointClass); It; ++It)
	{
		ASpawnPoint* Found = *It;
		if (!IsValid(Found)) continue;

		const FVector SpawnLocation = Found->GetActorLocation();
		
		// Box 범위 안에 들어있는 경우만 추가
		if (SpawnBounds.IsInside(SpawnLocation))
		{
			SpawnPointArray.Add(Found);
		}
	}

	return MoveTemp(SpawnPointArray);
}

