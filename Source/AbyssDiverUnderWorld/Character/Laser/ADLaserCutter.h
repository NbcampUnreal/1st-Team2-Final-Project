#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADLaserCutter.generated.h"

class UEquipableComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADLaserCutter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADLaserCutter();

#pragma region Method
public:
	UFUNCTION(NetMulticast, Reliable)
	void M_SetupVisibility(bool bIs1P);
	void M_SetupVisibility_Implementation(bool bIs1P);

protected:


private:
#pragma endregion

#pragma region Variable
public:
	void SetupVisibility(bool bIs1P);

protected:
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneComp;
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equip")
	TObjectPtr<UEquipableComponent> EquipableComp;


private:

#pragma endregion

#pragma region Getter, Setteer
public:
	USkeletalMeshComponent* GetSkeletalMesh() { return SkeletalMeshComp; }
	UEquipableComponent* GetEquipableComponent() { return EquipableComp; }

#pragma endregion


};
