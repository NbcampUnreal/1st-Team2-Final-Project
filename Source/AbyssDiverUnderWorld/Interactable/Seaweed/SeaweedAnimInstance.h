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
    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    float PhysicsBlendAlpha = 1.0f;

    void SetPhysicsBlendAlpha(float Alpha);

};
