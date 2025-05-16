#pragma once

#include "CoreMinimal.h"
#include "Gimmic/Spawner/Spawner.h"
#include "MonsterSpawner.generated.h"

class AUnitBase;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMonsterSpawner : public ASpawner
{
	GENERATED_BODY()

public:
	AMonsterSpawner();

public:
	virtual void BeginPlay() override;

public:
	virtual void Spawn() override;

protected:
	void SubtractMonsterClass(TSubclassOf<AUnitBase> Class);
	void RemoveInValidMonsterClass();

protected:
	/** 소환할 몬스터 클래스, 최대 개체 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Monster Info")
	TMap<TSubclassOf<AUnitBase>, uint8> MonsterClass;

	/** 소환할 몬스터 최소 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MinMonsterSpawnCount;

	/** 소환할 몬스터 최대 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MaxMonsterSpawnCount;
	
};
