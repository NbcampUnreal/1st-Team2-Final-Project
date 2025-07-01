// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Seaweed/SeaweedAnimInstance.h"

void USeaweedAnimInstance::SetPhysicsBlendAlpha(float Alpha)
{
    PhysicsBlendAlpha = Alpha;

}

// SeaweedAnimInstance.cpp
void USeaweedAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (CurrentBlendTime < MaxHoldTime)
    {
        PhysicsBlendAlpha = 1.0f;
        CurrentBlendTime += DeltaSeconds;
    }
}

