#include "Monster/BT/Serpmare/BTTask_RotationToTarget.h"

#include "Monster/Monster.h"
#include "Character/UnderwaterCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RotationToTarget::UBTTask_RotationToTarget()
{
	NodeName = TEXT("Rotation To Target");
	bNotifyTick = true; 
	bNotifyTaskFinished= false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_RotationToTarget::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{	
	FBTRotationToTargetTask* TaskMemory = (FBTRotationToTargetTask*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AAIController>(Comp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->Monster.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	BlackboardKeyName = GetSelectedBlackboardKey();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_RotationToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTRotationToTargetTask* TaskMemory = (FBTRotationToTargetTask*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeyName));
	if (!IsValid(Player)) return;

	const FVector PlayerLocation = Player->GetActorLocation();
	const FVector CurrentLocation = TaskMemory->Monster->GetActorLocation();

	const FRotator CurrentRotation = TaskMemory->Monster->GetActorRotation();
	FRotator TargetRotation = (PlayerLocation - CurrentLocation).GetSafeNormal().Rotation();
	TargetRotation.Roll = 0.0f;
	TargetRotation.Pitch = 0.0f;
	
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationInterpSpeed);
	
	TaskMemory->Monster->SetActorRotation(NewRotation);
	
}
