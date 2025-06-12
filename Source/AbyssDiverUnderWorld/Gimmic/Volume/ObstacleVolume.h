#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "ObstacleVolume.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObstacleComponentBeginOverlap, AUnderwaterCharacter*, OverlappedPlayer);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AObstacleVolume : public ATriggerBox
{
	GENERATED_BODY()

public:
	AObstacleVolume();

public:
	virtual void BeginPlay() override;

public:
	FOnObstacleComponentBeginOverlap OnObstacleComponentBeginOverlapDelegate;

private:
	UFUNCTION()
	void OnComponentBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnComponentEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
	
};
