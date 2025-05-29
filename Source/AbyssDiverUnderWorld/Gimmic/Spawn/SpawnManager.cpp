#include "Gimmic/Spawn/SpawnManager.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
#include "Spawner/Limadon/LimadonSpawner.h"
#include "Spawner/Monster/MonsterSpawner.h"
#include "Spawner/Ore/OreSpawner.h"

ASpawnManager::ASpawnManager()
{

}

void ASpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// 월드 내에 존재하는 LimadonSpawner를 가져온다.
	// LimadonSpawner는 1개로 유일하기 때문에 사실상 단순 1회성 for문에 불과하다.
	for (ALimadonSpawner* Spawner : TActorRange<ALimadonSpawner>(GetWorld()))
	{
		if (IsValid(Spawner))
		{
			LimadonSpawner = Spawner;
			break;
		}
	}
}

void ASpawnManager::SpawnByGroup()
{
	if (SpawnerGroups.IsValidIndex(0))
	{
		SpawnerGroups[0].MonsterSpawner->Spawn();
		SpawnerGroups[0].OreSpawner->Spawn();
		LimadonSpawner->Spawn();
		SpawnerGroups.RemoveAt(0);

		LOG(TEXT("Begin Spawn"));
	}
}

