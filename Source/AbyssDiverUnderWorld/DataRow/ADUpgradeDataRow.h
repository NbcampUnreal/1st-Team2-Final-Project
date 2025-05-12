#pragma once


#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "ADUpgradeDataRow.generated.h"

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	A,
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FADUpgradeDataRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly)
	EUpgradeType UpgradeType;

	UPROPERTY(EditDefaultsOnly)
	int32 Grade;

	UPROPERTY(EditDefaultsOnly)
	int32 Price;

	UPROPERTY(EditDefaultsOnly)
	TArray<int32> StatFactors;
};
