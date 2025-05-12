// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryDDO.generated.h"

enum class EItemType : uint8;
UCLASS()
class ABYSSDIVERUNDERWORLD_API UInventoryDDO : public UDragDropOperation
{
	GENERATED_BODY()

#pragma region Method
public:

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Index;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType Type;
#pragma endregion
	
};
