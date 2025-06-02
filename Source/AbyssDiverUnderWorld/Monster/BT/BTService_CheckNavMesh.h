// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckNavMesh.generated.h"



UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_CheckNavMesh : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	

public:
	UBTService_CheckNavMesh();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Variable
public:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ReturnToNavMeshLocationKey;

#pragma endregion
};
