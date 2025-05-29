#pragma once

#include "CoreMinimal.h"
#include "Gimmic/Spawn/Spawner/Spawner.h"
#include "LimadonSpawner.generated.h"

class ALimadon;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ALimadonSpawner : public ASpawner
{
	GENERATED_BODY()

public:
	ALimadonSpawner();

public:
	virtual void Spawn() override;

protected:
	/** 소환할 리마돈 클래스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner|Limadon")
	TSubclassOf<ALimadon> LimadonClass;

	/** 소환할 몬스터 최소 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MinLimadonSpawnCount;

	/** 소환할 몬스터 최대 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	uint8 MaxLimadonSpawnCount;

private:
	TArray<FVector> OreSpawnedLocations;
	TArray<FVector> LimadonSpawnedLocations;
	float MaxLimadonSpawnDistance;
	
};
