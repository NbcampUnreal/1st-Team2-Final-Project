#include "Gimmic/Spawn/Spawner/Serpmare/SerpmareSpawner.h"

#include "Gimmic/Spawn/SpawnPoint/Serpmare/BigSerpmareSpawnPoint.h"
#include "Gimmic/Spawn/SpawnPoint/Serpmare/MiniSerpmareSpawnPoint.h"

#include "AbyssDiverUnderWorld.h"
#include "Monster/Serpmare/Serpmare.h"


ASerpmareSpawner::ASerpmareSpawner()
{
	static ConstructorHelpers::FClassFinder<ASerpmare> BP_MiniSerpmare(TEXT("/Game/_AbyssDiver/Blueprints/Boss/Serpmare/BP_Serpmare"));
	if (BP_MiniSerpmare.Succeeded())
	{
		MiniSerpmareGroup.SerpmareClass = BP_MiniSerpmare.Class;
	}

	static ConstructorHelpers::FClassFinder<ASerpmare> BP_BigSerpmare(TEXT("/Game/_AbyssDiver/Blueprints/Boss/Serpmare/BP_Big_Serpmare"));
	if (BP_BigSerpmare.Succeeded())
	{
		BigSerpmareGroup.SerpmareClass = BP_BigSerpmare.Class;
	}

	MiniSerpmareGroup.MinSerpmareSpawnCount = 3;
	MiniSerpmareGroup.MaxSerpmareSpawnCount = 5;
	
	BigSerpmareGroup.MinSerpmareSpawnCount = 2;
	BigSerpmareGroup.MaxSerpmareSpawnCount = 3;
}

void ASerpmareSpawner::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	// PostInitializeComponents는 블루프린트에서도 발동함
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	// 호스트만 사용
	if (HasAuthority() == false)
	{
		return;
	}

	MiniSerpmareGroup.SerpmareSpawnPoints = GetSpawnPoint(AMiniSerpmareSpawnPoint::StaticClass());
	BigSerpmareGroup.SerpmareSpawnPoints = GetSpawnPoint(ABigSerpmareSpawnPoint::StaticClass());
}

void ASerpmareSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void ASerpmareSpawner::Spawn()
{
	SpawnSerpmare(MiniSerpmareGroup);
	SpawnSerpmare(BigSerpmareGroup);
}

void ASerpmareSpawner::SpawnSerpmare(const FSerpmareGroup& SerpmareGroup)
{
	if (!IsValid(SerpmareGroup.SerpmareClass) ||
		SerpmareGroup.MaxSerpmareSpawnCount == 0 ||
		SerpmareGroup.SerpmareSpawnPoints.Num() == 0) return;

	// 소환할 Serpmare 개체 수를 추출한다. 만일 MinSpawnCount에 의해 0이 된다면 얼리 리턴한다.
	const uint8 SpawnCount = FMath::RandRange(SerpmareGroup.MinSerpmareSpawnCount, SerpmareGroup.MaxSerpmareSpawnCount);
	if (SpawnCount == 0) return;
	
	// 안전성을 위해 원본 배열을 복사하여 사용한다.
	TArray<ASpawnPoint*> SpawnPointsArray = SerpmareGroup.SerpmareSpawnPoints.Array();

	// SpawnCount만큼 Serpmare 스폰 로직을 호출한다.
	// MinDistance만큼 최소 거리를 보장하려 했으나 for문의 계산 로직을 최소화 하고자 Serpmare는 단순 Spawn 한다.
	for (int8 i =0; i < SpawnCount; ++i)
	{
		if (SpawnPointsArray.Num() == 0) break;
		
		const uint8 RandomIndex = FMath::RandRange(0, SpawnPointsArray.Num() - 1);
		
		GetWorld()->SpawnActor<ASerpmare>(SerpmareGroup.SerpmareClass,
					SpawnPointsArray[RandomIndex]->GetActorLocation(),
					SpawnPointsArray[RandomIndex]->GetActorRotation()
					);

		SpawnPointsArray.RemoveAt(RandomIndex);
	}
}
