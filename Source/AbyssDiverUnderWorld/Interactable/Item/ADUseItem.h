// Fill out your copyright notice in the Description page of Project Settings.

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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);

	UFUNCTION(NetMulticast, Reliable)
	void M_SetSkeletalMesh(USkeletalMesh* NewMesh);
	void M_SetSkeletalMesh_Implementation(USkeletalMesh* NewMesh);

	void SetItemInfo(FItemData& ItemInfo, bool bIsEquipMode);
	void UnEquipMode();
	void EquipMode();

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

private:
	// 물 표면 높이
	float WaterHeight = 300.0f;

	// 유체 저항
	float WaterLinearDamping = 4.0f;
	float WaterAngularDamping = 7.0f;
#pragma endregion
	
};
