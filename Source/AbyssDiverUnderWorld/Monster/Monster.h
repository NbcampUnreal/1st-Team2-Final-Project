// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitBase.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster.generated.h"

class USplineComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMonster : public AUnitBase
{
	GENERATED_BODY()


public:
	AMonster();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	FVector GetPatrolLocation(int32 Index) const;
	UFUNCTION(BlueprintCallable)
	int32 GetNextPatrolIndex(int32 CurrentIndex) const;


#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Patrol")
	TObjectPtr<USplineComponent> PatrolSpline;

#pragma endregion

#pragma region Getter, Setter
public:
	UFUNCTION(BlueprintCallable)
	USplineComponent* GetSplineComp() const;
#pragma endregion
};
