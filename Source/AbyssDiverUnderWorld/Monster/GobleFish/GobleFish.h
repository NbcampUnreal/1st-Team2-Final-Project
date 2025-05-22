// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"
#include "GobleFish.generated.h"

class AGFProjectile;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AGobleFish : public AMonster
{
	GENERATED_BODY()
	

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void FireProjectile();

	virtual void PlayAttackMontage() override;
#pragma endregion

#pragma region Variable
private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AGFProjectile> ProjectileClass;

#pragma endregion
};
