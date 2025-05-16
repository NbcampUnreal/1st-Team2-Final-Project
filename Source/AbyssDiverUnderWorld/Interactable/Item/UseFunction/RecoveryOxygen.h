// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UseStrategy.h"
#include "RecoveryOxygen.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API URecoveryOxygen : public UUseStrategy
{
	GENERATED_BODY()
	
public:
	virtual void Use(AActor* Target) override;
};
