#pragma once


#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "UpgradeDataRow.generated.h"

constexpr uint8 DefaultGrade = 1;

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	Gather,
	Oxygen,
	Speed,
	Light,
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FUpgradeDataRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	FUpgradeDataRow() : 
		UpgradeType(EUpgradeType::Max),
		Grade(0), // Default Grade가 1이므로 0은 Invalid
		Price(0),
		StatFactor(0)
	{
	};
	
	UPROPERTY(EditDefaultsOnly)
	EUpgradeType UpgradeType;

	UPROPERTY(EditDefaultsOnly)
	uint8 Grade;

	UPROPERTY(EditDefaultsOnly)
	int32 Price;

	UPROPERTY(EditDefaultsOnly)
	int32 StatFactor;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> UpgradeIcon;

	UPROPERTY(EditDefaultsOnly)
	FString UpgradeName;

	UPROPERTY(EditDefaultsOnly)
	FString UpgradeDescription;
};
