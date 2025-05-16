#include "Gimmic/Spawn/SpawnManager.h"
#include "AbyssDiverUnderWorld.h"
#include "Spawner/Monster/MonsterSpawner.h"
#include "Spawner/Ore/OreSpawner.h"

ASpawnManager::ASpawnManager()
{

}

void ASpawnManager::SpawnByGroup()
{
	if (SpawnerGroups.IsValidIndex(0))
	{
		SpawnerGroups[0].MonsterSpawner->Spawn();
		SpawnerGroups[0].OreSpawner->Spawn();
		SpawnerGroups.RemoveAt(0);

		LOG(TEXT("Begin Spawn"));
	}
}

