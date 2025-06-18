#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "EyeStalker.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AEyeStalker : public ABoss
{
	GENERATED_BODY()

public:
	AEyeStalker();

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetEyeOpenness(float Openness);

	UFUNCTION(BlueprintImplementableEvent)
	void SetTargetPlayer(AUnderwaterCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent)
	void SetDetectedState(bool bDetected);
	
};
