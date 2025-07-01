// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_LookAtTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monster/Monster.h"

UBTService_LookAtTarget::UBTService_LookAtTarget()
{
	NodeName = TEXT("Look At Target");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
	InterpSpeed = 5.0f;
}

void UBTService_LookAtTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor) return;

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIPawn) return;

	AMonster* Monster = Cast<AMonster>(AIPawn);
	if (!Monster) return;
	if (!Monster->IsAnimMontagePlaying())
	{
		FVector MonsterLocation = AIPawn->GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector DirectionToTarget = (TargetLocation - MonsterLocation).GetSafeNormal();

		FRotator MonsterCurrentRotation = AIPawn->GetActorRotation();

		FRotator TargetToRotation = DirectionToTarget.Rotation();
		TargetToRotation.Roll = 0.0f;

		FRotator NewRotation = FMath::RInterpTo(MonsterCurrentRotation, TargetToRotation, DeltaSeconds, InterpSpeed);

		AIPawn->SetActorRotation(NewRotation);
	}
}
