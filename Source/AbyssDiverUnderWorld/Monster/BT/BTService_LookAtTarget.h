// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_LookAtTarget.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_LookAtTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
	
public:
	UBTService_LookAtTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


#pragma region Variable
private:
	UPROPERTY(EditAnywhere, Category = "Rotate")
	float InterpSpeed;

	UPROPERTY(EditAnywhere, Category = "TargetActor")
	FBlackboardKeySelector TargetActorKey;

#pragma endregion

};
