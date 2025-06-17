#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/BossAIController.h"
#include "EnhancedBossAIController.generated.h"

enum class EPerceptionType : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AEnhancedBossAIController : public ABossAIController
{
	GENERATED_BODY()

public:
	AEnhancedBossAIController();

public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	void SetBlackboardPerceptionType(EPerceptionType InPerceptionType);
	void InitVariables();

protected:
	UFUNCTION()
	void OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus);

	virtual void OnSightPerceptionSuccess(AUnderwaterCharacter* Player);
	virtual void OnSightPerceptionFail();
	virtual void OnHearingPerceptionSuccess(AUnderwaterCharacter* Player);
	virtual void SetBloodDetectedState();
	virtual void OnDamagePerceptionSuccess(AUnderwaterCharacter* Player);
		
private:
	void BindToObstacleComponent();
	
	UFUNCTION()
	void OnObstacleComponentBeginOverlap(AUnderwaterCharacter* OverlappedPlayer);

protected:
	UPROPERTY(EditAnywhere)
	uint8 bIsPerceptionSight : 1;

	UPROPERTY(EditAnywhere)
	uint8 bIsPerceptionHearing : 1;

	UPROPERTY(EditAnywhere)
	uint8 bIsPerceptionDamage : 1;

	UPROPERTY(EditAnywhere)
	uint8 bIsAlienShark : 1 = false;

private:
	/** 플레이어를 감지한 경우 true 반환 */
	uint8 bIsDetectedPlayer : 1;

	/** 피를 감지한 경우 true 반환 */
	uint8 bIsDetectedBlood : 1;

	/** 플레이어에 의해 공격 받은 상태인 경우 true 반환 */
	uint8 bIsDamagedByPlayer : 1;

	/** Damaged 상태로 전이한지 일정 시간이 경과한 경우 true 반환 */
	uint8 bCanTransitionToDamagedState : 1;

	/** Perception 중 플레이어가 사라진 경우 true 반환 */
	uint8 bIsDisappearPlayer : 1;
	
	FTimerHandle DamageStateTransitionTimerHandle;
	FTimerHandle BloodDetectedTimerHandle;

	static const FName bHasSeenPlayerKey;
	static const FName PerceptionTypeKey;
	static const FName bHasDetectedPlayerKey;
	static const FName bIsChasingKey;
	static const FName bHasAttackedKey;
	static const FName bIsPlayerHiddenKey;
	static const FName BloodOccurredLocationKey;
	static const FName TargetPlayerKey;
	static const FName bIsChasingPlayerKey;
	static const FName bIsDetectBloodKey;
	static const FName bIsChasingBloodKey;

public:
	FORCEINLINE bool GetIsDetectedPlayer() const { return bIsDetectedPlayer; }
	FORCEINLINE bool GetIsDetectedBlood() const { return bIsDamagedByPlayer; }
	FORCEINLINE bool GetIsDamagedByPlayer() const { return bIsDamagedByPlayer; }
	FORCEINLINE bool GetIsDisappearPlayer() const { return bIsDisappearPlayer; }

	FORCEINLINE void SetPerceptionSight(bool bIsEnabled) { bIsPerceptionSight = bIsEnabled; }
	FORCEINLINE void SetPerceptionHearing(bool bIsEnabled) { bIsPerceptionHearing = bIsEnabled; }
	FORCEINLINE void SetPerceptionDamage(bool bIsEnabled) { bIsPerceptionDamage = bIsEnabled; }
	
};
