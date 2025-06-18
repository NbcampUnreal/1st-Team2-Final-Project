// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "ADUseItem.generated.h"

class UEquipableComponent;
class USphereComponent;
class AUnderwaterCharacter;
enum class EEnvironmentState : uint8;

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

	void SetItemInfo(FItemData& ItemInfo, bool bIsEquipMode, EEnvironmentState CurrentEnviromnent);
	void SetVariableValues(int32 InAmount, int32 InCurrentAmmo, int32 InReserveAmmo);

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equip")
	TObjectPtr<UEquipableComponent> EquipableComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> GravityCollisionComp;
	
private:
	uint8 bIsInInventory : 1 = false;
	uint8 bIsOverlap : 1 = false;
	uint8 bIsEquip : 1 = false;

	FVector SpawnedItemGravity;

#pragma endregion	
#pragma region Getter, Setter
public:
	UEquipableComponent* GetEquipableComponent() { return EquipableComp; }
	// 유효하지 않으면 0.0f 반환
	float GetMeshMass() const;

#pragma endregion
};
