// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "ADUseItem.generated.h"

class UEquipableComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADUseItem : public AADItemBase
{
	GENERATED_BODY()
public:
	AADUseItem();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION(NetMulticast, Reliable)
	void M_SetSkeletalMesh(USkeletalMesh* NewMesh);
	void M_SetSkeletalMesh_Implementation(USkeletalMesh* NewMesh);

	UFUNCTION(NetMulticast, Reliable)
	void M_SetItemVisible(bool bVisible);
	void M_SetItemVisible_Implementation(bool bVisible);

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equip")
	TObjectPtr<UEquipableComponent> EquipableComp;
	
private:
	uint8 bIsInInventory : 1 = false;

#pragma endregion	
#pragma region Getter, Setter
public:
	UEquipableComponent* GetEquipableComponent() { return EquipableComp; }
	void SetItemVisiblity(bool InVisible);

#pragma endregion
};
