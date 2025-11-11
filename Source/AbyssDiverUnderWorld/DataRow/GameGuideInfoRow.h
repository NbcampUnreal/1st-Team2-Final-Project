// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameGuideInfoRow.generated.h"

UENUM(BlueprintType)
enum class EGameGuideType : uint8
{
	None        UMETA(DisplayName = "None"),
	Key       UMETA(DisplayName = "Basic"),
	Submit UMETA(DisplayName = "Submit"),
	Combat      UMETA(DisplayName = "Combat"),
	Mine      UMETA(DisplayName = "Mine"),
	Item       UMETA(DisplayName = "Item"),
	Charge       UMETA(DisplayName = "Charge")
};

USTRUCT(BlueprintType)
struct FGameGuideContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Content")
	FText ContentTitle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Content")
	TObjectPtr<UTexture2D> ContentImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Content")
	FText ContentDescription;
};

USTRUCT(BlueprintType)
struct ABYSSDIVERUNDERWORLD_API FGameGuideInfoRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Info")
	FName GuideTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Info")
	int32 GuideId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Info")
	EGameGuideType GuideType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Info")
	TArray<FGameGuideContent> GuideContents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Guide Info")
	uint8 bShouldBlink : 1;
};