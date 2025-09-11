#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "VignetteVolume.generated.h"

class UTimelineComponent;
class AUnderwaterCharacter;

UCLASS(BlueprintType, Blueprintable)
class ABYSSDIVERUNDERWORLD_API AVignetteVolume : public APostProcessVolume
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ApplyVignetteToPlayer(AUnderwaterCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent)
	void RemoveVignetteFromPlayer(AUnderwaterCharacter* Player);
	
	
};
