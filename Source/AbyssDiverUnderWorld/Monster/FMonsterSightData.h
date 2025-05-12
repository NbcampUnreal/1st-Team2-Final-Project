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
	float SightRadius; // Detection Range
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float LoseSightRadius; // Lose Interest Range
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float PeripheralVisionAngleDegrees; // VisionAngle Degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SenseInterval; // Perception info validiation Time

#pragma endregion
};