#include "Gimmic/Spawn/Spawner/Limadon/LimadonSpawner.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
#include "Boss/Limadon/Limadon.h"
#include "Gimmic/Spawn/SpawnPoint/Limadon/LimadonSpawnPoint.h"
#include "Interactable/Item/ADOreRock.h"

ALimadonSpawner::ALimadonSpawner()
{
	MinLimadonSpawnCount = 3;
	MaxLimadonSpawnCount = 5;
	MaxLimadonSpawnDistance = 1300.f;
	
	static ConstructorHelpers::FClassFinder<ALimadon> BP_Limadon(TEXT("/Game/_AbyssDiver/Blueprints/Boss/Limadon/BP_Limadon"));
	if (BP_Limadon.Succeeded())
	{
		LimadonClass = BP_Limadon.Class;
	}
}

void ALimadonSpawner::Spawn()
{
	// 1. 월드 내에 존재하는 광석의 위치를 추출하여 배열에 저장한다.
	for (AADOreRock* OreRock : TActorRange<AADOreRock>(GetWorld()))
	{
		if (IsValid(OreRock))
		{
			OreSpawnedLocations.Add(OreRock->GetActorLocation());
		}
	}

	// 2. 월드 내에 존재하는 Limadon SpawnPoint의 위치를 추출하여 배열에 저장한다.
	// 1페이즈에서 Limadon Spawn을 호출하지만 관계없는 2페이즈, 3페이즈의 Limadon SpawnPoint를 가져온다.
	// 하지만 거리 기반으로 스폰시키기 때문에 버그가 발생하진 않지만, 계산의 비효율성이 발생한다.
	for (ALimadonSpawnPoint* SpawnPoint : TActorRange<ALimadonSpawnPoint>(GetWorld()))
	{
		if (IsValid(SpawnPoint))
		{
			LimadonSpawnedLocations.Add(SpawnPoint);
		}
	}

	// 3. 스폰시킬 Limadon 개체 수를 추출한다.
	const uint8 LimadonSpawnCount = FMath::RandRange(MinLimadonSpawnCount, MaxLimadonSpawnCount);

	// 4. 스폰할 Limadon 개체 수만큼 Spawn 로직을 호출한다.
	// 월드 내에 스폰된 광석의 위치를 랜덤으로 가져온다.
	// 해당 광석의 위치에서 MaxLimadonSpawnDistance보다 가까운 Limadon SpawnPoint를 찾아 Limadon을 스폰시킨다.
	// 스폰 성공한 SpawnPoint는 월드 상에서 Destroy 한다. 이는 다음 Limadon Spawn 시의 효율성을 높이기 위함이다.
	for (int8 i = 0; i < LimadonSpawnCount; ++i)
	{
		if (OreSpawnedLocations.Num() == 0 || LimadonSpawnedLocations.Num() == 0)
			break;

		const uint8 RandomIndex = FMath::RandRange(0, OreSpawnedLocations.Num() - 1);
		const FVector& OreLocation = OreSpawnedLocations[RandomIndex];

		for (int32 j = 0; j < LimadonSpawnedLocations.Num(); ++j)
		{
			if (FVector::Dist(LimadonSpawnedLocations[j]->GetActorLocation(), OreLocation) < MaxLimadonSpawnDistance)
			{
				GetWorld()->SpawnActor<ALimadon>(
					LimadonClass,
					LimadonSpawnedLocations[j]->GetActorLocation(),
					LimadonSpawnedLocations[j]->GetActorRotation()
				);
				ALimadonSpawnPoint* CachedLimadonSpawnPoint = LimadonSpawnedLocations[j];
				LimadonSpawnedLocations.RemoveAt(j);
				CachedLimadonSpawnPoint->Destroy();
				break;
			}
		}
	}
}
