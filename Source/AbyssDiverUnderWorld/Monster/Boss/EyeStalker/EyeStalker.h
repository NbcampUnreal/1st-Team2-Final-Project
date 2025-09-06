#pragma once

#include "CoreMinimal.h"
#include "Monster/Boss/Boss.h"
#include "EyeStalker.generated.h"

class AVignetteVolume;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AEyeStalker : public AMonster
{
	GENERATED_BODY()

public:
	AEyeStalker();

public:
	UFUNCTION(NetMulticast, Reliable)
	void M_SetEyeOpenness(float Openness);
	void M_SetEyeOpenness_Implementation(float Openness);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetEyeOpenness(float Openness);

	UFUNCTION(NetMulticast, Reliable)
	void M_SetTargetPlayer(AUnderwaterCharacter* Player);
	void M_SetTargetPlayer_Implementation(AUnderwaterCharacter* Player);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetTargetPlayer(AUnderwaterCharacter* Player);

	UFUNCTION(NetMulticast, Reliable)
	void M_SetDetectedState(bool bDetected);
	void M_SetDetectedState_Implementation(bool bDetected);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetDetectedState(bool bDetected);
	
};
