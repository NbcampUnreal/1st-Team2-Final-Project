// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SeaweedAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USeaweedAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    float PhysicsBlendAlpha = 1.f;
    float CurrentBlendTime = 0.f;
    float MaxHoldTime = 2.f; // 이 시간동안은 강제적으로 1.0 유지
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    void SetPhysicsBlendAlpha(float Alpha);
};
