// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IADUsable.generated.h"

UINTERFACE(BlueprintType)
class UUsable : public UInterface
{
    GENERATED_BODY()
};

class IUsable
{
    GENERATED_BODY()

public:
    virtual void Use(AActor* Target) = 0;
};