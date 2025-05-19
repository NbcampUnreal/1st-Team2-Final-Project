#pragma once

#include "CoreMinimal.h"
#include "MissionData.generated.h"

USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

#pragma region Variable
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Stage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnlockHint;
#pragma endregion
};
