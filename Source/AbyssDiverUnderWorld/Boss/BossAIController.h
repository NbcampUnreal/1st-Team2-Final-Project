#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

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
	
protected:

private:
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	UBlackboardComponent* BlackboardComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Sight* SightConfig;

private:
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
