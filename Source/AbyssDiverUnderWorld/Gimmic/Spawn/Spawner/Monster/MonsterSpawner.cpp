#include "Gimmic/Spawn/Spawner/Monster/MonsterSpawner.h"
#include "AbyssDiverUnderWorld.h"
#include "Character/UnitBase.h"
#include "Gimmic/Spawn/SpawnPoint/SpawnPoint.h"
#include "Gimmic/Spawn/SpawnPoint/Monster/MonsterSpawnPoint.h"

AMonsterSpawner::AMonsterSpawner()
{
	MinMonsterSpawnCount = 5;
	MaxMonsterSpawnCount = 10;
}

void AMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	GetSpawnPoint(AMonsterSpawnPoint::StaticClass());
	RemoveInValidMonsterClass();
}

void AMonsterSpawner::Spawn()
{
	// 최소, 최대에 해당하는 몬스터 수 추출
	uint8 MonsterSpawnCount = FMath::RandRange(MinMonsterSpawnCount, MaxMonsterSpawnCount);
	MonsterSpawnCount = FMath::Clamp(MonsterSpawnCount, 0, TotalSpawnPoints.Num());

	for (int i = 0; i < MonsterSpawnCount; ++i)
	{
		// 유효한 스폰 포인트 탐색
		ASpawnPoint* SpawnPoint = GetPossibleSpawnPoint();
		if (!IsValid(SpawnPoint)) continue;

		// 소환할 수 있는 몬스터가 없으면 중단
		if (MonsterClass.Num() == 0) break;

		// 몬스터 클래스 중 랜덤하게 선택
		TArray<TSubclassOf<AUnitBase>> MonsterTypes;
		MonsterClass.GetKeys(MonsterTypes);

		// 소환할 몬스터 인덱스 추출
		uint8 RandomMonsterIndex = FMath::RandRange(0, MonsterClass.Num() - 1);

		LOG(TEXT("Spawn Success : %s"), *MonsterTypes[RandomMonsterIndex]->GetName());
		LOG(TEXT("Remain MonsterClass Num : %d"), MonsterClass.Num());

		// 랜덤 몬스터 스폰
		GetWorld()->SpawnActor<AUnitBase>(
					MonsterTypes[RandomMonsterIndex],
					SpawnPoint->GetActorLocation(),
					SpawnPoint->GetActorRotation()
					);

		// 해당 몬스터 클래스의 소환 가능한 개체 수 1 감소
		SubtractMonsterClass(MonsterTypes[RandomMonsterIndex]);
		
		// 스폰 성공 처리
		AddSpawnSuccessPoint(SpawnPoint);
		RemoveTotalSpawnPoint(SpawnPoint);
	}

	RemoveAllSpawnPoints();
}

void AMonsterSpawner::SubtractMonsterClass(TSubclassOf<AUnitBase> Class)
{
	if (!MonsterClass.Contains(Class)) return;

	if (--MonsterClass[Class] <= 0)
	{
		MonsterClass.Remove(Class);
	}
}

void AMonsterSpawner::RemoveInValidMonsterClass()
{ 
	for (auto It = MonsterClass.CreateIterator(); It; ++It)
	{
		if (It->Value <= 0)
		{
			It.RemoveCurrent();
		}
	}
}
