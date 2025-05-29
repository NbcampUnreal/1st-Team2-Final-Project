#include "Gimmic/Spawn/Spawner/Ore/OreSpawner.h"
#include "Gimmic/Spawn/SpawnPoint/SpawnPoint.h"
#include "Gimmic/Spawn/SpawnPoint/Ore/OreSpawnPoint.h"
#include "Interactable/Item/ADOreRock.h"

AOreSpawner::AOreSpawner()
{
	MinOreSpawnCount = 5;
	MaxOreSpawnCount = 10;
}

void AOreSpawner::BeginPlay()
{
	Super::BeginPlay();

	TotalSpawnPoints = GetSpawnPoint(AOreSpawnPoint::StaticClass());
}

void AOreSpawner::Spawn()
{
	// 소환할 광물 클래스가 유효하지 않은 경우 얼리 리턴
	if (!IsValid(OreRockClass)) return;
	
	// 최소 ~ 최대 광물 중 랜덤한 수 추출
	uint8 OreSpawnCount = FMath::RandRange(MinOreSpawnCount, MaxOreSpawnCount);

	// SpawnPoint 수를 넘어가면 안 되므로 Clamp
	OreSpawnCount = FMath::Clamp(OreSpawnCount, 0, TotalSpawnPoints.Num());

	for (int i=0; i<OreSpawnCount; ++i)
	{
		ASpawnPoint* SpawnPoint = GetPossibleSpawnPoint();
		if (!IsValid(SpawnPoint)) continue;
		
		GetWorld()->SpawnActor<AADOreRock>(OreRockClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
	}

	RemoveAllSpawnPoints();
}
