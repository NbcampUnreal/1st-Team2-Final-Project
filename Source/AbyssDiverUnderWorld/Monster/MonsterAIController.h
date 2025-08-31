// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "FMonsterSightData.h"
#include "Navigation/PathFollowingComponent.h"
#include "MonsterAIController.generated.h"

enum class EPerceptionType : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AMonsterAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;

#pragma region Method
protected:
	void LoadSightDataFromTable();
	
	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus); // Perception Callback Method


#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(BlueprintReadWrite, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI|SightData")
	TObjectPtr<UDataTable> SightDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class AMonster> Monster;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, float> LostActorsMap;

	UPROPERTY(EditDefaultsOnly, Category = "AI|SightData")
	FName MonsterID;

private:
	uint8 bIsLosingTarget : 1;
	float LostTargetTime;

	static const FName bIsChasingKey;
	static const FName TargetPlayerKey;
	static const FName PerceptionTypeKey;

#pragma endregion

#pragma region Getter, Setter
public:
	void SetbIsLosingTarget(bool IsLosingTargetValue);
	void SetBlackboardPerceptionType(EPerceptionType InPerceptionType);
#pragma endregion
};
