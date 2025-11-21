#pragma once

#include "CoreMinimal.h"

#include "BossAIController.h"
#include "Monster/Monster.h"

#include "Boss.generated.h"

class AEnhancedBossAIController;
class UNiagaraSystem;
enum class EBossPhysicsType : uint8;

class ATargetPoint;
class AUnderwaterCharacter;
class UAquaticMovementComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABoss : public AMonster
{
	GENERATED_BODY()
	
public:
	ABoss();

protected:

	virtual void BeginPlay() override;

#pragma region Method
public:

	/** 데미지를 받을 때 호출하는 함수 */
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayBloodEffect(const FVector& Location, const FRotator& Rotation);
	void M_PlayBloodEffect_Implementation(const FVector& Location, const FRotator& Rotation);

	/** 보스를 타겟 방향으로 회전시키는 함수 */
	virtual void RotationToTarget(AActor* Target);
	virtual void RotationToTarget(const FVector& InTargetLocation);

	/** 보스의 이동속도를 설정하는 함수 */
	void SetMoveSpeed(const float& SpeedMultiplier);

	virtual void M_OnDeath_Implementation() override;

private:
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	EBossPhysicsType BossPhysicsType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MinPatrolDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MaxPatrolDistance;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TraceDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TurnTraceDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float FourDirectionTraceDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bEnableDownTrace : 1 = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bEnableHorizontalTrace : 1 = true;

	UPROPERTY()
	FVector DamagedLocation;

	UPROPERTY()
	TObjectPtr<AEnhancedBossAIController> EnhancedAIController;

	FCollisionQueryParams Params;

private:

	float TurnTimer = 0.0f;
	float SphereOverlapRadius = 100.0f;
	FVector CachedSpawnLocation;
	FVector TurnDirection;

#pragma endregion

#pragma region Getter, Setter
public:

	FORCEINLINE UAnimInstance* GetAnimInstance() const { return AnimInstance; }
	FORCEINLINE FVector GetDamagedLocation() const { return DamagedLocation; }
	FORCEINLINE FVector GetCachedSpawnLocation() const { return CachedSpawnLocation; }

#pragma endregion
	
};