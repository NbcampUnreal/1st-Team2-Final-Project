// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitBase.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Monster/EMonsterState.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	FVector GetPatrolLocation(int32 Index) const;
	UFUNCTION(BlueprintCallable)
	int32 GetNextPatrolIndex(int32 CurrentIndex) const;
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayAttackMontage(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	void M_PlayAttackMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnDeath();

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
	TObjectPtr<ASplinePathActor> AssignedSplineActor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|AIController")
	TObjectPtr<AMonsterAIController> AIController;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimInstance> AnimInstance;
	UPROPERTY(Replicated, BlueprintReadWrite)
	EMonsterState MonsterState;
	

private:
	static const FName MonsterStateKey;
#pragma endregion

#pragma region Getter. Setter
public:
	UAnimMontage* GetAttackMontage() { return AttackMontage; }

	// Virtual function to get collision components for attack range determination externally
	virtual USphereComponent* GetAttackHitComponent() const { return nullptr; }

	void SetMonsterState(EMonsterState State);
};
