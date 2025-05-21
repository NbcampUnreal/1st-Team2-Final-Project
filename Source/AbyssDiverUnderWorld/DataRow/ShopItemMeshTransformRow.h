#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "ShopItemMeshTransformRow.generated.h"

USTRUCT(BlueprintType)
struct FShopItemMeshTransformRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 ItemId;

	UPROPERTY(EditDefaultsOnly)
	FRotator ReletiveRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly)
	FVector ReletivePosition = FVector(270, 0, 0);

	UPROPERTY(EditDefaultsOnly)
	FVector ReletiveSize = FVector::OneVector;
};
