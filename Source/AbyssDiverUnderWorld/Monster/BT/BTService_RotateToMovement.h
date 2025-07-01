// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_RotateToMovement.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_RotateToMovement : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	

public:
	UBTService_RotateToMovement();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Variable
	UPROPERTY(EditAnywhere)
	float InterpSpeed;

#pragma endregion
};
