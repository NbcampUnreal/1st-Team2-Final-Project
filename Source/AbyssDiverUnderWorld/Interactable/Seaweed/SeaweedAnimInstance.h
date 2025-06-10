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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BendAlpha = 0.f;
};
