// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_RotateToTarget.h"
#include "AIController.h"
#include "Monster/Monster.h"

UBTTask_RotateToTarget::UBTTask_RotateToTarget()
{
	NodeName = "RotateToTarget";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RotateToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
	if (!AIPawn) return EBTNodeResult::Failed;

	CachedTargetActor = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject("TargetActorTest"));
	if (!CachedTargetActor || !IsValid(CachedTargetActor)) return EBTNodeResult::Failed;

	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ACharacter* AIPawn = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!AIPawn || !CachedTargetActor) return;

	FVector ToTarget = CachedTargetActor->GetActorLocation() - AIPawn->GetActorLocation();
	ToTarget.Normalize();

	FRotator CurrentRot = AIPawn->GetActorRotation();
	FRotator TargetRot = FRotationMatrix::MakeFromX(ToTarget).Rotator();
	TargetRot.Roll = 0.f;
	// TargetRot.Pitch = 0.f;

	// Rotational interpolation
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, 5.f);
	AIPawn->SetActorRotation(NewRot);

	// Reach check
	float Angle = FMath::Acos(FVector::DotProduct(AIPawn->GetActorForwardVector(), ToTarget)) * (180.f / PI);
	if (Angle < 10.f)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}