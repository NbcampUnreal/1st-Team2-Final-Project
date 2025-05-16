// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitBase.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Monster.generated.h"

class ASplinePathActor;
class USphereComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMonster : public AUnitBase
{
	GENERATED_BODY()


public:
	AMonster();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	FVector GetPatrolLocation(int32 Index) const;
	UFUNCTION(BlueprintCallable)
	int32 GetNextPatrolIndex(int32 CurrentIndex) const;
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayAttackMontage(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	void M_PlayAttackMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);


#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
	TObjectPtr<ASplinePathActor> AssignedSplineActor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

#pragma endregion

#pragma region Getter. Setter
public:
	UAnimMontage* GetAttackMontage() { return AttackMontage; }
	// Virtual function to get collision components for attack range determination externally
	virtual USphereComponent* GetAttackHitComponent() const { return nullptr; }
};
