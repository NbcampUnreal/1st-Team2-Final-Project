// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FADItemDataRow.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Equipment = 0,
	Consumable = 1,
	Exchangable = 2,
	Battery = 3,
	Bullet = 4,
	Max = 5 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FFADItemDataRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Quantity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Weight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Stackable : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Thumbnail;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SpawnActor;
};
