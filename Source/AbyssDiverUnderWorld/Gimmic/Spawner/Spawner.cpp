#include "Gimmic/Spawner/Spawner.h"
#include "Gimmic/SpawnPoint/SpawnPoint.h"
#include "EngineUtils.h"

ASpawner::ASpawner()
{
	MinDistance = 100.0f;
	MaxTries = 100;
}

void ASpawner::BeginPlay()
{
	Super::BeginPlay();

	for (ASpawnPoint* SpawnPoint : TActorRange<ASpawnPoint>(GetWorld()))
	{
		if (IsValid(SpawnPoint))
		{
			TotalSpawnPoints.Add(SpawnPoint);
		}
	}
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
			if (IsValid(UsedPoint)) continue;

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

