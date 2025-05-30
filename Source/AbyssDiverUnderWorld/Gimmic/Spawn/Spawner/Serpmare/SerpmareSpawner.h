#pragma once

#include "CoreMinimal.h"
#include "Gimmic/Spawn/Spawner/Spawner.h"
#include "SerpmareSpawner.generated.h"

class ABigSerpmareSpawnPoint;
class AMiniSerpmareSpawnPoint;
class ASerpmare;

USTRUCT(BlueprintType)
struct FSerpmareGroup
{
	GENERATED_BODY()
	
public:
	/** 소환할 Serpmare 클래스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner|Class")
	TSubclassOf<ASerpmare> SerpmareClass;

	/** 최소 Serpmare 소환 개체 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MinSerpmareSpawnCount;

	/** 최대 Serpmare 소환 개체 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MaxSerpmareSpawnCount;

	/** Serpmare 스폰 포인트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	TSet<TObjectPtr<ASpawnPoint>> SerpmareSpawnPoints;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASerpmareSpawner : public ASpawner
{
	GENERATED_BODY()

public:
	ASerpmareSpawner();

public:
	virtual void BeginPlay() override;

public:
	virtual void Spawn() override;

private:
	void SpawnSerpmare(const FSerpmareGroup& SerpmareGroup);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Big Serpmare")
	FSerpmareGroup BigSerpmareGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Mini Serpmare")
	FSerpmareGroup MiniSerpmareGroup;
	
};
