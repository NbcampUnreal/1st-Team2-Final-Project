#pragma once

#include "CoreMinimal.h"
#include "BossAIController.h"
#include "Character/UnitBase.h"
#include "Boss.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABoss : public AUnitBase
{
	GENERATED_BODY()
	
public:
	ABoss();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	
protected:

private:
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Blackboard")
	UBlackboardComponent* BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	ABossAIController* AIController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<UAnimMontage*> NormalAttackAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<UAnimMontage*> SpecialAttackAnimations;

private:
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
