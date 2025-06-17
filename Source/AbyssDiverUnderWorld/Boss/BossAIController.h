#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BossAIController.generated.h"

class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
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
	void SetSightRadius(float InRadius);
	void SetVisionAngle(float Angle);
	bool IsStateSame(EBossState State);
	EPathFollowingRequestResult::Type MoveToActorWithRadius(AActor* TargetActor);
	EPathFollowingRequestResult::Type MoveToLocationWithRadius(const FVector& Location);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float DetectedStateInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Radius")
	float MoveToActorAcceptanceRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Radius")
	float MoveToLocationAcceptanceRadius;

	UPROPERTY()
	TObjectPtr<ABoss> Boss;

	FTimerHandle DetectedStateTimerHandle;
	
private:
	static const FName BossStateKey;
	
#pragma endregion

#pragma region Getter, Setter

#pragma endregion
	
};