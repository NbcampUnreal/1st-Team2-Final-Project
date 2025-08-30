#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterDexRow.generated.h"


USTRUCT(BlueprintType)
struct FMonsterDexRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine))
	FText Features;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine))
	FText Weakness;
};