// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ButtonDataRow.generated.h"

USTRUCT(BlueprintType)
struct FButtonDataRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ButtonTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> ButtonMesh;
};