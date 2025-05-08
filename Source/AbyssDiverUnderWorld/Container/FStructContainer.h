#pragma once

#include "CoreMinimal.h"
#include "FStructContainer.generated.h"

USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) 
    FName Name;

    UPROPERTY(EditAnywhere) 
    uint8 Id;

    UPROPERTY(EditAnywhere) 
    uint8 Quantity;

    UPROPERTY(EditAnywhere) 
    int32 Amount;

    UPROPERTY(EditAnywhere)
    float Mass;
};