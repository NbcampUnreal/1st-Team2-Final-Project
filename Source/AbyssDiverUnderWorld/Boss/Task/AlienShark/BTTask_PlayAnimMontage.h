#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_PlayAnimMontage.generated.h"

USTRUCT(BlueprintType)
struct FBlackboardKeyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KeyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive;

	FBlackboardKeyInfo()
	{
		KeyName = TEXT("");
		bIsActive = false;
	}
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_PlayAnimMontage : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_PlayAnimMontage();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	UPROPERTY(EditAnywhere)
	float AnimMontageLength = 1.5f;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditAnywhere)
	FBlackboardKeyInfo BlackboardKeyInfo;

	UPROPERTY()
	float AccumulatedTime = 0.0f;
	
};
