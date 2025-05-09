#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

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
	void SetDefaultVisionAngle();
	void SetChasingVisionAngle();
	
protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(NetMulticast, Reliable)
	void M_AddDetectedPlayer(AActor* Target);
	void M_AddDetectedPlayer_Implementation(AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
	void M_RemoveDetectedPlayer(AActor* Target);
	void M_RemoveDetectedPlayer_Implementation(AActor* Target);

private:
#pragma endregion

#pragma region Variable
public:

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
	TArray<TObjectPtr<AActor>> DetectedPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float DefaultVisionAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float ChasingVisionAngle;

private:
	static const FName BossStateKey;
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
