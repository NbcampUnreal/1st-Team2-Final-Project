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
    uint8 bIsUnlocked : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnlockHint;

    FORCEINLINE bool operator==(const FMissionData& Other) const
    {
        return Title == Other.Title;
    }
#pragma endregion
};

FORCEINLINE uint32 GetTypeHash(const FMissionData& Data)
{
    return GetTypeHash(Data.Title);
}
