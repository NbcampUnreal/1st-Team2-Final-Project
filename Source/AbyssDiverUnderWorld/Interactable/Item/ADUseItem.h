﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "ADUseItem.generated.h"

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

	UFUNCTION(NetMulticast, Reliable)
	void M_UnEquipMode();
	void M_UnEquipMode_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void M_EquipMode();
	void M_EquipMode_Implementation();

	void SetItemInfo(FItemData& ItemInfo, bool bIsEquipMode);
	void SetVariableValues(int32 InAmount, int32 InCurrentAmmo, int32 InReserveAmmo);


#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

#pragma endregion	
};
