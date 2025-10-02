// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FMonsterSightData.generated.h"


USTRUCT(BlueprintType)
struct FMonsterSightData : public FTableRowBase
{
	GENERATED_BODY()

#pragma region Variable
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SightRadius; // Perception Detect 반경
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float LoseSightRadius; // Perception Lost 반경
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float PeripheralVisionAngleDegrees; // 시야각

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float PointOfViewBackwardOffset; // Perception 시작 위치 뒤로 당기는 정도

#pragma endregion
};