// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "ADFiringWeapon.generated.h"

class USkeletalMeshComponent;
class AADProjectileBase;
class USceneComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADFiringWeapon : public AADItemBase
{
	GENERATED_BODY()
public:
	AADFiringWeapon();

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void Fire();
	void Reload();

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AADProjectileBase> BulletClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

#pragma endregion
	
};
