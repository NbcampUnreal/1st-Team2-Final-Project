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
	void SetBlackboardPerceptionType(EPerceptionType PerceptionType);
	void InitPerceptionInfo();
	void InitBlackboardVariables();

protected:
	UFUNCTION()
	void OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus);

protected:
	/** 시야에서 플레이어가 사라진 이후 플레이어를 기억하는 최대 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="AI|Stat")
	float TargetSaveTime;

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

	EPerceptionType PerceptionType;

	FTimerHandle DamageStateTransitionTimerHandle;
	FTimerHandle TargetSaveTimerHandle;

	static const FName bPerceptionUpdatedKey;
	static const FName bHasSeenPlayerKey;
	static const FName PerceptionTypeKey;
	static const FName bHasDetectedPlayerKey;
	
public:
	FORCEINLINE bool GetIsDetectedPlayer() const { return bIsDetectedPlayer; }
	FORCEINLINE bool GetIsDetectedBlood() const { return bIsDamagedByPlayer; }
	FORCEINLINE bool GetIsDamagedByPlayer() const { return bIsDamagedByPlayer; }
	FORCEINLINE bool GetIsDisappearPlayer() const { return bIsDisappearPlayer; }
	FORCEINLINE EPerceptionType GetPerceptionType() const { return PerceptionType; }
	FORCEINLINE void SetPerceptionType(EPerceptionType InPerceptionType) { PerceptionType = InPerceptionType; }
};
