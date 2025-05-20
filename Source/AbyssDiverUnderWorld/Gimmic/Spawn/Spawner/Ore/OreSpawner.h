#pragma once

#include "CoreMinimal.h"
#include "Gimmic/Spawn/Spawner/Spawner.h"
#include "OreSpawner.generated.h"

class AADOreRock;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AOreSpawner : public ASpawner
{
	GENERATED_BODY()

public:
	AOreSpawner();

public:
	virtual void BeginPlay() override;

public:
	virtual void Spawn() override;

protected:
	/** 소환할 광물 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Mesh")
	TSubclassOf<AADOreRock> OreRockClass;
	
	/** 최소 광물 소환 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MinOreSpawnCount;

	/** 최대 광물 소환 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MaxOreSpawnCount;
	
	
};
