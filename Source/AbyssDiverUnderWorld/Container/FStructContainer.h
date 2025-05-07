// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FStructContainer.generated.h"

USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FName Name;

    UPROPERTY(EditAnywhere) int8 Id;

    UPROPERTY(EditAnywhere) int8 Quantity;

    UPROPERTY(EditAnywhere) int16 Amount;
};