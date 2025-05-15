// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "ADUseItem.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API AADUseItem : public AADItemBase
{
	GENERATED_BODY()
public:
	AADUseItem();

#pragma region Method
public:

	UFUNCTION(NetMulticast, Reliable)
	void M_SetSkeletalMesh(USkeletalMesh* NewMesh);
	void M_SetSkeletalMesh_Implementation(USkeletalMesh* NewMesh);

	void SetItemInfo(FItemData& ItemInfo);

#pragma endregion

#pragma region Variable
protected:
	TObjectPtr<USceneComponent> Scene;

	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
#pragma endregion
	
};
