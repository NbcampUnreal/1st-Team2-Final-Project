#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BossAIController.generated.h"

enum class EBossState : uint8;
class AUnderwaterCharacter;
class ABoss;
struct FAIStimulus;
class UAISenseConfig_Sight;
class UPawnSensingComponent;
class UBehaviorTreeComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABossAIController : public AAIController
{
	GENERATED_BODY()


public:
	ABossAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

#pragma region Method
public:
	void SetVisionAngle(float& Angle);
	bool IsStateSame(EBossState State);
	EPathFollowingRequestResult::Type MoveToActorWithRadius();
	EPathFollowingRequestResult::Type MoveToLocationWithRadius(const FVector& Location);
	
protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(NetMulticast, Reliable)
	void M_AddDetectedPlayer(AUnderwaterCharacter* Target);
	void M_AddDetectedPlayer_Implementation(AUnderwaterCharacter* Target);

	UFUNCTION(NetMulticast, Reliable)
	void M_RemoveDetectedPlayer(AUnderwaterCharacter* Target);
	void M_RemoveDetectedPlayer_Implementation(AUnderwaterCharacter* Target);

private:
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float DefaultVisionAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float ChasingVisionAngle;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Target")
	TArray<TObjectPtr<AUnderwaterCharacter>> DetectedPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float DetectedStateInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Radius")
	float MoveToActorAcceptanceRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Radius")
	float MoveToLocationAcceptanceRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Perception")
	uint8 bIsSightDetectionPossible : 1;

	UPROPERTY()
	TObjectPtr<ABoss> Boss;
	
private:
	static const FName BossStateKey;
	uint8 bIsDetectedStatePossible : 1;
	float AccumulatedTime;
	FTimerHandle DetectedStateTimerHandle;
#pragma endregion

#pragma region Getter, Setter
public:
	void SetDetectedStatePossible();

#pragma endregion
	
};