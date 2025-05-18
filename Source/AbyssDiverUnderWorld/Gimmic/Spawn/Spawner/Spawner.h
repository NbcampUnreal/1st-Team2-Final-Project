#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

class UBoxComponent;
class ASpawnPoint;

UCLASS(Abstract)
class ABYSSDIVERUNDERWORLD_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawner();

public:
	virtual void BeginPlay() override;

public:
	/** 순수 가상 함수 */
	virtual void Spawn() PURE_VIRTUAL(ASpawn::Spawn, ;);

protected:
	ASpawnPoint* GetPossibleSpawnPoint();
	void RemoveAllSpawnPoints();
	void GetSpawnPoint(TSubclassOf<ASpawnPoint> SpawnPointClass);

protected:
	/** 스폰 포인트를 포함할 영역 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	TObjectPtr<UBoxComponent> LocationRange;
	
 	/** 최대로 시도할 루프 카운트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Stat")
	int32 MaxTries;
	
	/** 맵에 배치된 모든 스폰 포인트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner|Points")
	TSet<TObjectPtr<ASpawnPoint>> TotalSpawnPoints;
	
	/** 스폰되는 개체 간의 최소 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|MinDistance")
	float MinDistance;

private:
	UPROPERTY()
	TSet<TObjectPtr<ASpawnPoint>> SpawnSuccessPoints;

public:
	/** 스폰 성공한 지점 정보 추가 */
	FORCEINLINE void AddSpawnSuccessPoint(ASpawnPoint* SpawnPoint){SpawnSuccessPoints.Add(SpawnPoint);}

	/** 스폰 성공한 지점 제거 */
	FORCEINLINE void RemoveTotalSpawnPoint(ASpawnPoint* SpawnPoint)
	{
		if (TotalSpawnPoints.Contains(SpawnPoint))
		{
			TotalSpawnPoints.Remove(SpawnPoint);
		}
	}
};
