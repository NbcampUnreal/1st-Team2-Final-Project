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
class UAquaticMovementComponent;
class AUnderwaterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterDeadSignature, AActor*, Killer, AMonster*, DeadMonster);

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
	virtual void SetNewTargetLocation();
	virtual void PerformNormalMovement(const float& InDeltaTime);
	virtual void PerformChasing(const float& InDeltaTime);

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

	UFUNCTION()
	void OnAttackCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAttackCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	void ApplyPhysicsSimulation();
	void HandleSetting_OnDeath();

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ChasingRotationSpeedMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ChasingMovementSpeedMultiplier = 2.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MovementInterpSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float RotationInterpSpeed = 1.1f;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnMonsterDeadSignature OnMonsterDead;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	uint8 bIsChasing : 1;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	/** 새로운 물리 기반 수중 이동 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Monster|Movement")
	TObjectPtr<UAquaticMovementComponent> AquaticMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Monster|Target")
	TObjectPtr<AUnderwaterCharacter> TargetPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Monster|Target")
	TObjectPtr<AUnderwaterCharacter> CachedTargetPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Monster|Collision")
	TObjectPtr<UCapsuleComponent> AttackCollision;

protected:
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WanderRadius = 1300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinTargetDistance = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float CurrentMoveSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bDrawDebugLine : 1 = false;

	FCollisionQueryParams Params;
	uint8 bIsTurning : 1 = false;
	uint8 bIsAttacking : 1 = false;

	static const FName MonsterStateKey;
	static const FName InvestigateLocationKey;
	static const FName PatrolLocationKey;
	static const FName TargetActorKey;

private:
	FVector TargetLocation;
	FVector DesiredTargetLocation; // 새로운 목표 위치 (보간 전)
	FVector InterpolatedTargetLocation; // 보간된 목표 위치
	float TargetLocationInterpSpeed = 2.0f;  // 타겟 위치 보간 속도
	float PatrolInterpSpeed = 1.5f; // 순찰 시 더 부드러운 보간 속도
	uint8 bIsAttackCollisionOverlappedPlayer : 1;

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

	// @ TODO : PerformAttack Task빌드를 위해 임시로 가져온 Getter. Monster에는 AttackCollision이 없음. Boss에만 있다.
	FORCEINLINE bool GetIsAttackCollisionOverlappedPlayer() const { return bIsAttackCollisionOverlappedPlayer; };

	FORCEINLINE void SetTargetLocation(const FVector& InTargetLocation) { TargetLocation = InTargetLocation; }
	FORCEINLINE void InitTarget() { TargetPlayer = nullptr; };
	FORCEINLINE AUnderwaterCharacter* GetCachedTarget() const { return CachedTargetPlayer; };
	FORCEINLINE void SetCachedTarget(AUnderwaterCharacter* Target) { CachedTargetPlayer = Target; };
	FORCEINLINE void InitCachedTarget() { CachedTargetPlayer = nullptr; };
	FORCEINLINE AUnderwaterCharacter* GetTarget() const { return TargetPlayer; };
	FORCEINLINE void SetTarget(AUnderwaterCharacter* Target) { TargetPlayer = Target; };
};
