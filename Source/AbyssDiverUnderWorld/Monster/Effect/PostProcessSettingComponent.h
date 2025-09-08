#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PostProcessSettingComponent.generated.h"

class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UPostProcessSettingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPostProcessSettingComponent();

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(Client, Unreliable)
	void C_ActivateVignetteEffect();
	void C_ActivateVignetteEffect_Implementation();

	UFUNCTION(Client, Unreliable)
	void C_DeactivateVignetteEffect();
	void C_DeactivateVignetteEffect_Implementation();

private:
	void ActivateVignetteEffect();
	void DeactivateVignetteEffect();
	void UpdateVignetteIntensity();

protected:
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

private:
	/** Vignette 효과 관련 변수 */
	UPROPERTY(meta = (ClampMin = "0"))
	uint8 VignetteReferenceCount = 0;
	FTimerHandle VignetteTimerHandle;
	float StartVignetteIntensity = 0.0f;
	float TargetVignetteIntensity = 0.0f;
	float CurrentTransitionTime = 0.0f;
	float TransitionDuration = 1.0f;
	uint8 bIsTransitioning : 1 = false;
		
};
