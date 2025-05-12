// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ADPlayerState.generated.h"

class UADInventoryComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AADPlayerState();

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UADInventoryComponent> InventoryComp;
#pragma endregion
	
#pragma region Getter/Setter
public:
	UADInventoryComponent* GetInventory() { return InventoryComp; };
#pragma endregion
};
