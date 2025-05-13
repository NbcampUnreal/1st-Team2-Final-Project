// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interface/IADUsable.h"
#include "RecoveryOxygen.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API URecoveryOxygen : public UObject, public IUsable
{
	GENERATED_BODY()
	
public:
	virtual void Use_Implementation(AActor* Target) override;
};
