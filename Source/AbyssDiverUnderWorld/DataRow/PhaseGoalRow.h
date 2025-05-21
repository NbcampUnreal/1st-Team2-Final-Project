#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PhaseGoalRow.generated.h"


UENUM(BlueprintType)
enum class EMapName : uint8
{
	test1  UMETA(DisplayName = "Shallow"),
	test2  UMETA(DisplayName = "level2"),
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FPhaseGoalRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMapName MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Phase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoalCredit;

};

USTRUCT(BlueprintType)
struct FMapPathDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EMapName MapName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString MapPath;
};