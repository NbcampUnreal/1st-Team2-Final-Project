// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MapDepthRow.generated.h"

enum class EMapName : uint8;

/** 맵의 깊이 구역을 정의하는 열거형 */
UENUM()
enum EDepthZone : uint8
{
	SafeZone UMETA(DisplayName = "Safe Zone"),
	WarningZone UMETA(DisplayName = "Warning Zone"),
	DangerZone UMETA(DisplayName = "Danger Zone"),
	Max UMETA(Hidden),
};

/**
 * 맵의 깊이 정보를 저장하는 Table Row 구조체
 * Z 변수는 게임 맵에서의 실제 Z 좌표를 기준으로 한다
 * Depth 변수는 특정 지점에서의 게임적 깊이를 나타낸다.
 */
USTRUCT(BlueprintType)
struct ABYSSDIVERUNDERWORLD_API FMapDepthRow : public FTableRowBase
{
	GENERATED_BODY()

	FMapDepthRow();
	
	/** 깊이 정보를 저장할 Map 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMapName MapName;

	/** 깊이 정보를 저장할 Map 이름 문자열 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MapNameString;

	/** 게임 맵에서 기준이 되는 Z 좌표 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReferenceZ;

	/** ReferenceZ의 깊이 값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReferenceDepth;

	/** 경고 구역 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bUseWarningZone : 1;

	/** 위험 구역 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bUseDangerZone : 1;
	
	/** 경고 구역의 Z축 값, World Map을 기준으로 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float WarningZoneZ;

	/** 위험 구역의 Z축 값, World Map을 기준으로 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DangerZoneZ;
};
