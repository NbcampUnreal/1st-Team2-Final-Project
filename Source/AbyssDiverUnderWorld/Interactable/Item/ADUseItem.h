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

#pragma region Method
public:
	void SetItemInfo(FItemData& ItemInfo);
#pragma endregion

#pragma region Variable
protected:
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
#pragma endregion
	
};
