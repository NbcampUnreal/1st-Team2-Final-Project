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
	float SightRadius; // 탐지 범위
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float LoseSightRadius; // 어그로 해제 시작 범위
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float PeripheralVisionAngleDegrees; // 시야 각도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SenseInterval; // 지각 정보 유효 시간

#pragma endregion
};