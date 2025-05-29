#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnManager.generated.h"

class ALimadonSpawner;
class AMonsterSpawner;
class AOreSpawner;

USTRUCT(BlueprintType)
struct FSpawnerGroup
{
	GENERATED_BODY()

public:
	// 광물 스포너
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TObjectPtr<AOreSpawner> OreSpawner;

	// 몬스터 스포너
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TObjectPtr<AMonsterSpawner> MonsterSpawner;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawnManager();

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnByGroup();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FSpawnerGroup> SpawnerGroups;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner")
	TObjectPtr<ALimadonSpawner> LimadonSpawner;

};
