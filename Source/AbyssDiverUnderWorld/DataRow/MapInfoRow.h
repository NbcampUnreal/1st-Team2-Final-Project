// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapInfoRow.generated.h"

enum class EMapName : uint8;

USTRUCT(BlueprintType)
struct FMapInfoRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMapName MapEnumType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MapId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MapDescription;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> MapThumbnail;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsUnlocked : 1;
};