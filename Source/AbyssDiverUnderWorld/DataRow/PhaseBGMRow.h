#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataRow/SoundDataRow/BGMDataRow.h"
#include "PhaseBGMRow.generated.h"

USTRUCT(BlueprintType)
struct FPhaseBGMRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) 
    FName   MapName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) 
    int32   Phase;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) 
    ESFX_BGM BGM;  
};