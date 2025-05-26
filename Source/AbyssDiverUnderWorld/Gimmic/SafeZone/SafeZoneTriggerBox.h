#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "SafeZoneTriggerBox.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASafeZoneTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	ASafeZoneTriggerBox();

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
	
};
