// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ADPlayerState.generated.h"

class UADInventoryComponent;
class UUpgradeComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AADPlayerState();

protected:

	virtual void BeginPlay() override;
	virtual void PostNetInit() override;

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UADInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUpgradeComponent> UpgradeComp;

#pragma endregion
	
#pragma region Getter/Setter
public:
	FORCEINLINE UADInventoryComponent* GetInventory() const { return InventoryComp; };
	FORCEINLINE UUpgradeComponent* GetUpgradeComp() const { return UpgradeComp; };

#pragma endregion
};
