// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitBase.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Monster/MonsterSoundComponent.h"
#include "Monster/EMonsterState.h"
#include "Monster.generated.h"

class ASplinePathActor;
class USphereComponent;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterDeadSignature, AActor*, Killer, AMonster*, DeadMonster);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMonster : public AUnitBase
{
	GENERATED_BODY()


public:
	AMonster();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	FVector GetPatrolLocation(int32 Index) const;
	UFUNCTION(BlueprintCallable)
	int32 GetNextPatrolIndex(int32 CurrentIndex) const;
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayMontage(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	void M_PlayMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);

	UFUNCTION(NetMulticast, Reliable)
	void M_SpawnBloodEffect(FVector Location, FRotator Rotation);
	void M_SpawnBloodEffect_Implementation(FVector Location, FRotator Rotation);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void OnDeath();
	UFUNCTION(NetMulticast, Reliable)
	void M_OnDeath();
	virtual void M_OnDeath_Implementation();

	virtual void PlayAttackMontage();
	void UnPossessAI();
	virtual void NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor);
	
	UFUNCTION()
	void AddDetection(AActor* Actor);
	
	UFUNCTION()
	void RemoveDetection(AActor* Actor);
	
	UFUNCTION()
	void ForceRemoveDetection(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	bool IsAnimMontagePlaying() const;

	UFUNCTION()
	void DelayDestroyed();

	UFUNCTION()
	void MonsterRaderOff();

protected:
	void ApplyPhysicsSimulation();
	void RotateToTarget(float DeltaTime);
	void RotateToMovementForward(float DeltaTime);

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnMonsterDeadSignature OnMonsterDead;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	uint8 bIsChasing : 1;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
	TObjectPtr<ASplinePathActor> AssignedSplineActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|AIController")
	TObjectPtr<AMonsterAIController> AIController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|SoundComponent")
	TObjectPtr<UMonsterSoundComponent> MonsterSoundComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimInstance> AnimInstance;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimMontage> CurrentAttackAnim;
	UPROPERTY(Replicated, BlueprintReadWrite)
	EMonsterState MonsterState;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|AttackAnimation")
	TArray<TObjectPtr<UAnimMontage>> AttackAnimations;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|DetectedAnimation")
	TObjectPtr<UAnimMontage> DetectedAnimations;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|HitReactAnimation")
	TObjectPtr<UAnimMontage> HitReactAnimations;
	UPROPERTY(EditDefaultsOnly, Category = "AI|FX")
	TObjectPtr<UNiagaraSystem> BloodEffect;
	UPROPERTY(EditAnywhere, Category = "AI")
	float ChaseTriggerTime;
	UPROPERTY(EditAnywhere, Category = "AI")
	float ChaseSpeed;
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolSpeed;
	UPROPERTY(EditAnywhere, Category = "AI")
	float InvestigateSpeed;
	UPROPERTY(EditAnywhere, Category = "AI")
	float FleeSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<AActor*, int32> DetectionRefCounts;

	static const FName MonsterStateKey;
	static const FName InvestigateLocationKey;
	static const FName PatrolLocationKey;
	static const FName TargetActorKey;
	
#pragma endregion

#pragma region Getter. Setter
public:
	TArray<UAnimMontage*> GetAttackAnimations() { return AttackAnimations; }
	
	UFUNCTION(BlueprintCallable)
	EMonsterState GetMonsterState() { return MonsterState; }
	virtual void SetMonsterState(EMonsterState NewState);
	void SetMaxSwimSpeed(float Speed);
	int32 GetDetectionCount() const;


	// Virtual function to get collision components for attack range determination externally
	virtual USphereComponent* GetAttackHitComponent() const { return nullptr; }
	
};
