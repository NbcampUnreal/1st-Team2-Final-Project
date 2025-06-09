#include "Boss/Task/Serpmare/BTTask_RotationToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Character/UnderwaterCharacter.h"

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

	TaskMemory->AIController = Cast<ABossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

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
	const FVector CurrentLocation = TaskMemory->Boss->GetActorLocation();

	const FRotator CurrentRotation = TaskMemory->Boss->GetActorRotation();
	FRotator TargetRotation = (PlayerLocation - CurrentLocation).GetSafeNormal().Rotation();
	TargetRotation.Roll = 0.0f;
	TargetRotation.Pitch = 0.0f;

	LOG(TEXT("Rotation"));
	
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationInterpSpeed);
	
	TaskMemory->Boss->SetActorRotation(NewRotation);
	
}
