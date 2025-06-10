// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipableComponent.generated.h"

class USkeletalMesh;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UEquipableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipableComponent();

protected:
	virtual void BeginPlay() override;
#pragma region Method
public:

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(VisibleAnywhere, Category = "Equip")
	TWeakObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Equip", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh>   StaticMeshAsset;
	UPROPERTY(EditAnywhere, Category = "Equip", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset; 

#pragma endregion

#pragma region Getter, Setter	

	UMeshComponent* GetMeshComponent() const { return MeshComponent.Get(); }
	UStaticMesh* GetStaticMeshAsset()   const { return StaticMeshAsset; }
	USkeletalMesh* GetSkeletalMeshAsset() const { return SkeletalMeshAsset; }
	bool IsSkeletal() const;
	bool IsStatic()   const;
};
